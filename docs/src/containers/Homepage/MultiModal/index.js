import React from "react";

export default function MultiModal() {
  return (
    <div className="container">
      <div className="text-center mb-10">
        <h2>Multi-modal</h2>
        <p className="mt-2">
          Nitro integrates best-of-class open source AI libraries
        </p>
      </div>

      <div className="grid grid-cols-2 lg:grid-cols-4 gap-4">
        <div className="dark:bg-[#27272A]/20 bg-[#D4D4D8]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
          <img
            src="/img/multi-modals/ic-think.svg"
            alt="Icon Think"
            className="mb-2"
          />
          <h6 className="font-medium text-xl">Think</h6>
          <p className="mt-2 font-medium">Llama2, Mistral, CausalML,...</p>
        </div>
        <div className="dark:bg-[#27272A]/20 bg-[#D4D4D8]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
          <img
            src="/img/multi-modals/ic-imagine.svg"
            alt="Icon Imagine"
            className="mb-2"
          />
          <h6 className="font-medium text-xl">Imagine</h6>
          <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm text-white">
            Coming Soon
          </p>
        </div>
        <div className="dark:bg-[#27272A]/20 bg-[#D4D4D8]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
          <img
            src="/img/multi-modals/ic-vision.svg"
            alt="Icon Vision"
            className="mb-2"
          />
          <h6 className="font-medium text-xl">Vision</h6>
          <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm text-white">
            Coming Soon
          </p>
        </div>
        <div className="dark:bg-[#27272A]/20 bg-[#D4D4D8]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
          <img
            src="/img/multi-modals/ic-speech.svg"
            alt="Icon Speech"
            className="mb-2"
          />
          <h6 className="font-medium text-xl">Speech</h6>
          <p className="mt-2 font-medium bg-blue-600 inline-flex px-2 rounded-md text-sm text-white">
            Coming Soon
          </p>
        </div>
      </div>
    </div>
  );
}
