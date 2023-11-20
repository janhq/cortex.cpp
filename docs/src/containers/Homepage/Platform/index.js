import React from "react";

export default function Platform() {
  const options = ["platforms", "CPUs", "GPUs"];

  return (
    <div className="container">
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-10 items-center text-center">
        <div
          className="border border-gray-800 p-4 lg:p-6 rounded-lg"
          style={{
            background:
              "radial-gradient(500px 500px at 50% 100%, #172554, transparent)",
          }}
        >
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
          <div
            className="relative border border-gray-800 p-4 lg:p-6 rounded-lg h-full "
            style={{
              background:
                "radial-gradient(300px 300px at 50% 100%, #172554, transparent)",
            }}
          >
            <h5>GPUs</h5>
            <div className="relative">
              <img
                src="/img/platforms/cross-gpus.svg"
                alt="Element GPUS"
                className="mt-4 mx-auto"
              />
            </div>
          </div>
          <div
            className="border border-gray-800 p-4 lg:p-6 rounded-lg h-full"
            style={{
              background:
                "radial-gradient(300px 300px at 50% 100%, #172554, transparent)",
            }}
          >
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
