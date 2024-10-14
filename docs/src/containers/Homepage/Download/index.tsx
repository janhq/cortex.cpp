import CardDownload from "./CardDownload";
import { usePluginData } from "@docusaurus/useGlobalData";

const DownloadSection = () => {
  const latestRelease = usePluginData("latest-release");

  return (
    <div className="container">
      <div className="mt-24 text-center">
        <h2 className="text-4xl font-grotesk">Download</h2>
      </div>
      <div className="my-10">
        <CardDownload lastRelease={latestRelease} />
      </div>
    </div>
  );
};

export default DownloadSection;
