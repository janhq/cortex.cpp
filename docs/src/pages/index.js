import React, { Fragment } from "react";

import useDocusaurusContext from "@docusaurus/useDocusaurusContext";

import Layout from "@theme/Layout";

import DownloadButton from "@site/src/containers/DownloadButton";

import GetNitro from "@site/src/containers/Homepage/GetNitro";
import Statistic from "@site/src/containers/Homepage/Statistic";
import OpenAI from "@site/src/containers/Homepage/OpenAI";
import Platform from "@site/src/containers/Homepage/Platform";
import MultiModal from "@site/src/containers/Homepage/MultiModal";
import BuiltWithNitro from "@site/src/containers/Homepage/BuiltWithNitro";
import Architecture from "@site/src/containers/Homepage/Architecture";
import Banner from "@site/src/containers/Banner";

export default function Home() {
  const { siteConfig } = useDocusaurusContext();
  return (
    <Fragment>
      <Banner />
      <Layout title={`${siteConfig.tagline}`} description="About Nitro.">
        <img
          src="/img/elements/ellipse.png"
          alt="Element Hero Ellipse"
          className="absolute top-0 w-full left-0 opacity-30 dark:visible invisible"
        />
        <img
          src="/img/elements/lines.svg"
          alt="Element Lines"
          className="absolute w-full top-0 left-0 object-cover h-[460px] opacity-50 lg:opacity-80"
        />
        <main className="relative z-30">
          <div className="container py-10">
            <div className="text-center">
              <h1 className="bg-gradient-to-b dark:from-white from-black to-gray-700 dark:to-gray-400 bg-clip-text text-6xl lg:text-8xl font-semibold leading-tight text-transparent dark:text-transparent lg:leading-tight">
                Embeddable AI
              </h1>
              <p className="text-2xl mt-1">
                A fast, lightweight&nbsp;
                <span className="py-0.5 px-2 bg-indigo-600 rounded-full text-base font-bold dark:text-black text-white">
                  3mb
                </span>
                &nbsp;inference server to supercharge apps with local AI.
              </p>

              <div className="mt-10">
                <DownloadButton />
              </div>
            </div>
            <div className="mt-10">
              <GetNitro />
            </div>
          </div>

          <div className="mt-4">
            <OpenAI />
          </div>

          <div className="mt-10">
            <Statistic />
          </div>

          <div className="mt-10">
            <Platform />
          </div>

          <div className="mt-20">
            <MultiModal />
          </div>

          <div className="mt-20">
            <BuiltWithNitro />
          </div>

          <div className="my-20">
            <Architecture />
          </div>
        </main>
      </Layout>
    </Fragment>
  );
}
