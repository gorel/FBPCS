/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <filesystem>
#include <fstream>

#include <folly/Format.h>
#include <folly/Random.h>
#include <folly/init/Init.h>
#include <folly/logging/xlog.h>
#include <gflags/gflags.h>
#include <string>

#include "../common/FilepathHelpers.h"
#include "../common/S3CopyFromLocalUtil.h"
#include "AttributionIdSpineCombinerOptions.h"
#include "AttributionIdSpineFileCombiner.h"
#include "AttributionIdSpineCombinerUtil.h"
#include "fbpcf/aws/AwsSdk.h"
#include "fbpcf/io/FileManagerUtil.h"
#include "fbpcf/io/IInputStream.h"

// TODO Task: T93622832
#include "../common/CostEstimation.h"


int main(int argc, char** argv) {

  measurement::private_attribution::CostEstimation cost {"data_processing"};
  cost.start();

  folly::init(&argc, &argv);
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  fbpcf::AwsSdk::aquire();

  std::filesystem::path outputPath{FLAGS_output_path};
  std::filesystem::path tmpDirectory{FLAGS_tmp_directory};

  XLOG(INFO) << "Starting data_prcessing run on: data_path:" << FLAGS_data_path
             << ", spine_path: " << FLAGS_spine_path
             << ", output_path: " << FLAGS_output_path
             << ", tmp_directory: " << FLAGS_tmp_directory;

  auto dataInStreamPtr = fbpcf::io::getInputStream(FLAGS_data_path);
  auto spineInStreamPtr = fbpcf::io::getInputStream(FLAGS_spine_path);
  std::istream& dataStream = dataInStreamPtr->get();
  std::istream& spineStream = spineInStreamPtr->get();

  // Get a random ID to avoid potential name collisions if multiple
  // runs at the same time point to the same input file
  auto randomId = std::to_string(folly::Random::secureRand64());
  std::string tmpFilename = randomId + "_" +
      private_lift::filepath_helpers::getBaseFilename(outputPath);
  std::filesystem::path tmpFilepath = (tmpDirectory / tmpFilename);
  XLOG(INFO) << "Writing temporary file to " << tmpFilepath;
  std::ofstream tmpFile{tmpFilepath};

  pid::combiner::attributionIdSpineFileCombiner(
      dataStream, spineStream, tmpFile);
  tmpFile.close();

  auto outputType = fbpcf::io::getFileType(outputPath);
  if (outputPath != tmpFilepath) {
    if (outputType == fbpcf::io::FileType::S3) {
      private_lift::s3_utils::uploadToS3(tmpFilepath, outputPath);
    } else if (outputType == fbpcf::io::FileType::Local) {
      std::filesystem::copy(
          tmpFilepath,
          outputPath,
          std::filesystem::copy_options::overwrite_existing);
    } else {
      throw std::runtime_error{"Unsupported output destination"};
    }

    // We need to make sure we clean up the tmpfiles now
    std::remove(tmpFilepath.c_str());
  }

  cost.end();
  XLOG(INFO) << cost.getEstimatedCostString();
  if (FLAGS_run_name != ""){
    std::string runName = folly::to<std::string>(
                              cost.getApplication(), "_",
                              FLAGS_run_name, "_",
                              measurement::private_attribution::getDateString());
    XLOG(INFO) << cost.writeToS3(runName, cost.getEstimatedCostDynamic(runName));
  }

  return 0;
}
