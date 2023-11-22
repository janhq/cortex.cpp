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
          {/* Temporary disabled */}
          {/* <div className="dark:bg-[#27272A] bg-[#F4F4F5] inline-flex rounded-lg overflow-hidden mb-4 border dark:border-gray-800 border-gray-300 justify-center">
            <ul className="flex">
              {tabs.map((option, i) => {
                return (
                  <li
                    key={i}
                    className={twMerge(
                      "capitalize px-4 py-2 cursor-pointer font-bold",
                      activeTab === option && "bg-blue-600 text-white"
                    )}
                    onClick={() => setActiveTab(option)}
                  >
                    {option}
                  </li>
                );
              })}
            </ul>
          </div> */}

          <h2>Lightweight</h2>
          <div className="w-full lg:w-1/2 mx-auto">
            <p className="mt-2">
              {lightweightTabActive
                ? "Nitro is an extremely lightweight library built for app developers to run local AI"
                : "Nitro is built using Drogon, a blazing-fast C++ server and natively implements batch inference, multi-threading and more"}
            </p>
          </div>
          {lightweightTabActive ? (
            <div className="grid grid-cols-3 mt-8 gap-4 lg:gap-8">
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl dark:text-yellow-400 text-yellow-600">
                  Nitro
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  3mb
                </h6>
              </div>
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl text-blue-600">
                  Local AI
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  193mb
                </h6>
              </div>
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl text-red-600">Ollama</h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  332mb
                </h6>
              </div>
            </div>
          ) : (
            <div className="grid grid-cols-3 mt-8 gap-4 lg:gap-8">
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl dark:text-yellow-400 text-yellow-600">
                  Nitro
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="dark:text-gray-400 text-gray-800">
                  token/s
                </span>
              </div>
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl text-red-600">
                  Llama.cpp (base)
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="dark:text-gray-400 text-gray-800">
                  token/s
                </span>
              </div>
              <div className="dark:bg-[#27272A]/20 bg-[#E4E4E7]/20 border dark:border-gray-800 border-gray-300 p-4 lg:p-6 rounded-lg">
                <h6 className="font-medium lg:text-xl text-blue-600">
                  Text-inference
                </h6>
                <h6 className="mt-2 font-medium text-2xl sm:text-3xl lg:text-5xl">
                  24
                </h6>
                <span className="dark:text-gray-400 text-gray-800">
                  token/s
                </span>
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
