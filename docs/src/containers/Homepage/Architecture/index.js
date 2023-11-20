import React from "react";

export default function Architecture() {
  return (
    <div className="container">
      <div className="text-center mb-10">
        <h2>Nitro's Architecture</h2>
        <p className="mt-2 w-full lg:w-2/5 mx-auto">
          Nitro is 100% open source and licensed under the AGPLv3 license. We
          build upon the shoulders of giants at llama.cpp and Drogon.
        </p>
      </div>
      <div className="grid lg:grid-cols-12 text-center relative">
        <img
          src="/img/elements/stars.svg"
          alt="Element Stars"
          className="absolute top-0 w-full left-0"
        />
        <div className="col-span-full lg:col-span-10 lg:col-start-2 relative z-10">
          <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
            <p>OpenAI-compatible API</p>
          </div>
          <div className="border border-gray-800 p-4 lg:p-6 rounded-lg mt-8">
            <h5 className="mb-6">Nitro</h5>
            <div className="grid grid-cols-1 lg:grid-cols-4 gap-4">
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <p>Authentication</p>
                <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm">
                  Coming Soon
                </p>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <p>Batching</p>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <p>Multi-threading</p>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <p>Model Management</p>
              </div>
            </div>
          </div>

          <div className="border border-gray-800 p-4 lg:p-6 rounded-lg mt-8">
            <h5 className="mb-6">Model Engines</h5>
            <div className="grid grid-cols-1 lg:grid-cols-3 gap-4">
              <div className="border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <h6 className="mb-4">LLMs</h6>
                <div className="grid grid-cols-1 xl:grid-cols-2 gap-4">
                  <div className="bg-[#27272A]/50 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                    <p>Llama.cpp</p>
                  </div>
                  <div className="bg-[#27272A]/50 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                    <p>TensorRT-LLM</p>
                    <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm">
                      Coming Soon
                    </p>
                  </div>
                </div>
              </div>
              <div className="border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <h6 className="mb-4">Speech</h6>
                <div className="bg-[#27272A]/50 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                  <p>Whisper.cpp</p>
                  <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm">
                    Coming Soon
                  </p>
                </div>
              </div>
              <div className="border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                <h6 className="mb-4">Vision</h6>
                <div className="bg-[#27272A]/50 border border-gray-800 p-4 lg:p-6 rounded-lg backdrop-blur-md">
                  <p>StableDiffusion</p>
                  <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm">
                    Coming Soon
                  </p>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
