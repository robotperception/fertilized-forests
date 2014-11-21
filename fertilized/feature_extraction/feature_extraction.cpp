/* Author: Christoph Lassner. */
#include "./feature_extraction.h"
#include "./feature_extraction_vision.h"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vector>
#include <algorithm>

#include "../ndarray.h"

#include "../global.h"
#include "../types.h"
#include "../util/gil.hpp"
#include "./hog_extractor.h"

namespace ft = fertilized::vision::features::feature_channels;
#ifdef PYTHON_ENABLED
namespace py = boost::python;
#endif

namespace fertilized {
    ft::HOGExtractor HOG_EXTRACTOR = ft::HOGExtractor();

    DllExport std::vector<cv::Mat> extract_hough_forest_features(
                                                const cv::Mat &image,
                                                const bool &full) {
      std::vector<cv::Mat> res(0);
      if (full) {
        HOG_EXTRACTOR.extractFeatureChannels32(image, res);
      } else {
        HOG_EXTRACTOR.extractFeatureChannels15(image, res);
      }
      return res;
    };

    DllExport Array<uint8_t, 3, 3> extract_hough_forest_features(
                                                const Array<uint8_t const, 3, 3> &image,
                                                const bool &full) {
      if (image.TPLMETH getSize<2>() != 3) {
        throw Fertilized_Exception("The image must be a 3-channel color image!");
      }
      size_t n_channels = 15;
      if (full) {
        n_channels = 32;
      }
      // Construct the result array.
      Array<uint8_t, 3, 3> result = allocate(makeVector(n_channels,
                                                        image.getSize<0>(),
                                                        image.getSize<1>()));
      // Checks done. The rest can happen in parallel.
      {
#ifdef PYTHON_ENABLED
        py::gil_guard_release gil_guard;
#endif
        // Construct an OpenCV mat.
        const cv::Mat im_cvm = cv::Mat(static_cast<int>(image.getSize<0>()),
                                       static_cast<int>(image.getSize<1>()),
                                       CV_8UC3,
                                       reinterpret_cast<void*>(const_cast<uint8_t*>(image.getData())));
        std::vector<cv::Mat> resvec = extract_hough_forest_features(im_cvm,
                                                                    full);
        // Copy the results to the array.
        uint8_t *resptr = reinterpret_cast<uint8_t*>(result.getData());
        for (int i = 0; i < n_channels; ++i) {
          std::copy(resvec[i].data, resvec[i].dataend,
                    resptr + i * image.getSize<0>() * image.getSize<1>());
        }
      }
      return result;
    };
}  // namespace fertilized