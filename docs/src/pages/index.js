import React from "react";
import Dropdown from "@site/src/components/Elements/dropdown";
import useDocusaurusContext from "@docusaurus/useDocusaurusContext";

import useBaseUrl from "@docusaurus/useBaseUrl";
import Layout from "@theme/Layout";
import AnnoncementBanner from "@site/src/components/Announcement";
import {
  CloudArrowUpIcon,
  CursorArrowRaysIcon,
  ShieldCheckIcon,
  CpuChipIcon,
  ClipboardDocumentIcon,
  CubeTransparentIcon,
  ComputerDesktopIcon,
  FolderPlusIcon,
} from "@heroicons/react/24/outline";

import ThemedImage from "@theme/ThemedImage";

// const features = [
//   {
//     name: "Personal AI that runs on your computer",
//     desc: "Jan runs directly on your local machine, offering privacy, convenience and customizability.",
//     icon: ComputerDesktopIcon,
//   },
//   {
//     name: "Private and offline, your data never leaves your machine",
//     desc: "Your conversations and data are with an AI that runs on your computer, where only you have access.",
//     icon: ShieldCheckIcon,
//   },
//   {
//     name: "No subscription fees, the AI runs on your computer",
//     desc: "Say goodbye to monthly subscriptions or usage-based APIs. Jan runs 100% free on your own hardware.",
//     icon: CubeTransparentIcon,
//   },
//   {
//     name: "Extendable via App and Plugin framework",
//     desc: "Jan has a versatile app and plugin framework, allowing you to customize it to your needs.",
//     icon: FolderPlusIcon,
//   },
// ];

export default function Home() {
  const { siteConfig } = useDocusaurusContext();
  return (
    <>
      {/* <AnnoncementBanner /> */}
      <Layout title={`${siteConfig.tagline}`} description="About Nitro.">
        <main className="relative">
          <div className="container py-10">
            <div className="text-center">
              <h1 className="bg-gradient-to-b dark:from-white from-black to-gray-500 dark:to-gray-400 bg-clip-text text-6xl lg:text-8xl font-bold leading-tight text-transparent dark:text-transparent lg:leading-tight">
                Embeddable AI
              </h1>
              <p className="text-2xl">
                A fast, lightweight (3mb) inference server to supercharge apps
                with local AI
              </p>
            </div>
          </div>
        </main>
      </Layout>
    </>
  );
}
