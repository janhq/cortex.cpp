import { Button } from "@site/src/components/Button";

import Link from "@docusaurus/Link";

import Announcement from "@site/src/components/Announcement";

import { FaGithub } from "react-icons/fa";
import DropdownDownload from "@site/src/components/DropdownDownload";
import { usePluginData } from "@docusaurus/useGlobalData";

const SimpleHeroSection = () => {
  const latestRelease = usePluginData("latest-release");
  return (
    <div className="container">
      <div className="text-center mb-10">
        <Announcement />
      </div>

      <div className="text-center">
        <div className="w-full lg:w-1/2 mx-auto">
          <h1 className="text-4xl lg:text-6xl font-grotesk leading-tight">
            Run and Customize Local LLMs
          </h1>
        </div>
        <p className="text-xl w-full mx-auto lg:w-2/3 text-black/60 dark:text-white/60">
          Powers <span className="text-black dark:text-white">👋</span> Jan
        </p>

        <div className="mt-8 flex gap-8 justify-center items-center">
          <DropdownDownload lastRelease={latestRelease} />
          <Link href="/contact" target="_blank">
            <Button theme="secondary">Get in Touch</Button>
          </Link>
        </div>
      </div>

      <div className="relative w-full md:w-1/2 mx-auto mt-1 py-14 pb-20">
        <div
          className="rounded-lg border-neutral-800 border border-solid w-full bg-neutral-900 overflow-hidden flex flex-col"
          style={{
            boxShadow:
              "0px 0px 0px 0.5px rgba(255, 255, 255, 0.20), 0px 5px 12px 0px rgba(0, 0, 0, 0.50), 0px 16px 40px 0px rgba(0, 0, 0, 0.46)",
          }}
        >
          <div className="flex border-b border-neutral-700 bg-neutral-800 w-full py-3">
            <div className="h-3 w-3 bg-red-500 mx-1 ml-2 rounded-full" />
            <div className="h-3 w-3 bg-yellow-500 mx-1 rounded-full" />
            <div className="h-3 w-3 bg-green-500 mx-1 rounded-full" />
          </div>
          <div className="w-full">
            <div className="p-4 text-left">
              <code className="bg-transparent border-none inline-block">
                <p className="text-neutral-500 mb-0"># Run Local LLMs</p>
                <p className="mb-0">
                  <span className="text-green-600">cortex&nbsp;</span>
                  <span className="text-white">run&nbsp;</span>
                  <span className="text-cyan-600">llama3.1</span>
                </p>
                <p className="mb-0">
                  <span className="text-green-600">cortex&nbsp;</span>
                  <span className="text-white">run&nbsp;</span>
                  <span className="text-cyan-600">llama3.1:tensorrt-llm</span>
                </p>
                <p className="mb-0">
                  <span className="text-green-600">cortex&nbsp;</span>
                  <span className="text-white">run&nbsp;</span>
                  <span className="text-cyan-600">llama3.1:onnx</span>
                </p>
              </code>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};
export default SimpleHeroSection;
