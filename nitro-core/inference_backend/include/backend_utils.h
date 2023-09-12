#pragma once
#include "inference_lib.h"

namespace backend_utils {
inline void finished_upload_action(
    const Aws::S3::S3Client *s3Client,
    const Aws::S3::Model::PutObjectRequest &request,
    const Aws::S3::Model::PutObjectOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &context) {
  if (outcome.IsSuccess()) {
    LOG_INFO << "Success: Finished uploading ";
  } else {
    LOG_ERROR << "Error: Failed uploading the image file error message : "
              << outcome.GetError().GetMessage();
  }
}

template <typename T>
inline void
post_process_inference(std::string response_filename,
                       std::vector<T> output_data, std::string s3_bucket,
                       std::string s3_public_endpoint,
                       inference_aws::S3::S3Client &s3Client,
                       const std::vector<int> &default_output_shape) {
  std::vector<uchar> imageBuffer;

  TIME_IT(inference::process_image(output_data, default_output_shape),
          imageBuffer, "Time taken to process vector to byte: ");

  auto imageByteBuffer =
      inference_aws::MakeShared<inference_aws::StringStream>("UploadTag");
  TIME_IT(
      {
        std::copy(imageBuffer.begin(), imageBuffer.end(),
                  std::ostream_iterator<char>(*imageByteBuffer));
      },
      "Time taken to process vector to byte: ");

  inference_aws::S3::Model::PutObjectRequest request;
  request.WithBucket(s3_bucket).WithKey(response_filename);
  request.SetBody(imageByteBuffer);

  TIME_IT(s3Client.PutObjectAsync(request, finished_upload_action),
          "Async cost time for upload; ");
}
} // namespace backend_utils