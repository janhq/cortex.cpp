import React from "react";

export default function Platform() {
  return (
    <div className="container">
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-10 items-center text-center">
        <div className="radial-blur-lg border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
          <h2>Cross-Platform</h2>
          <p className="mt-2">
            Nitro runs on cross-platform on CPU and GPU architectures
          </p>
          <div className="relative mt-8">
            <img
              src="/img/platforms/cross-platforms.svg"
              alt="Element Platforms"
              className="mx-auto"
            />
          </div>
        </div>

        <div className="flex flex-col h-full gap-y-8">
          <div className="radial-blur-sm relative border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg h-full ">
            <h5>GPUs</h5>
            <div className="relative">
              <img
                src="/img/platforms/cross-gpus.svg"
                alt="Element GPUS"
                className="mt-4 mx-auto"
              />
            </div>
          </div>
          <div className="radial-blur-sm border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg h-full">
            <h5>CPUs</h5>
            <div className="relative">
              <img
                src="/img/platforms/cross-cpus.svg"
                alt="Element CPUS"
                className="mt-4 mx-auto"
              />
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
