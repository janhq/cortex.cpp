import React, { useState } from "react";

import { twMerge } from "tailwind-merge";

const tabs = ["lightweight", "fast"];

export default function Statistic() {
  const [activeTab, setActiveTab] = useState("lightweight");

  const lightweightTabActive = activeTab === "lightweight";

  return (
    <div className="container">
      <div className="grid grid-cols-12 text-center">
        <div className="col-span-full lg:col-span-12 pt-4">
          <div className="bg-[#27272A] inline-flex rounded-lg overflow-hidden mb-4 border border-gray-800 justify-center">
            <ul className="flex">
              {tabs.map((option) => {
                return (
                  <li
                    className={twMerge(
                      "capitalize px-4 py-2 cursor-pointer",
                      activeTab === option && "bg-blue-600"
                    )}
                    onClick={() => setActiveTab(option)}
                  >
                    {option}
                  </li>
                );
              })}
            </ul>
          </div>
          <div className="w-full lg:w-1/2 mx-auto">
            <p className="mt-1">
              {lightweightTabActive
                ? "Nitro is an extremely lightweight library built for app developers to run local AI"
                : "Nitro is built using Drogon, a blazing-fast C++ server and natively implements batch inference, multi-threading and more"}
            </p>
          </div>
          {lightweightTabActive ? (
            <div className="grid lg:grid-cols-3 mt-8 gap-4 lg:gap-8">
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-yellow-400">Nitro</h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  3mb
                </h6>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-red-600">Ollama</h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  332mb
                </h6>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-blue-600">Local AI</h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  193mb
                </h6>
              </div>
            </div>
          ) : (
            <div className="grid lg:grid-cols-3 mt-8 gap-4 lg:gap-8">
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-yellow-400">Nitro</h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="text-gray-400">token/s</span>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-red-600">
                  Llama.cpp (base)
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="text-gray-400">token/s</span>
              </div>
              <div className="bg-[#27272A]/20 border border-gray-800 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium text-xl text-blue-600">
                  Text-inference
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="text-gray-400">token/s</span>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
