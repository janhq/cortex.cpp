import { RiStarSFill } from "react-icons/ri";
import { usePluginData } from "@docusaurus/useGlobalData";
import { useDiscordWidget } from "@site/src/hooks/useDiscordWidget";
import { formatCompactNumber } from "@site/src/utils";
import Link from "@docusaurus/Link";

const SocialNavbar = () => {
  const repoInfo: {
    stargazers_count?: number;
  } = usePluginData("repo-info");
  const { data: discordWidget } = useDiscordWidget();

  return (
    <div className="lg:flex hidden mr-4 items-center gap-x-2">
      <div className="header-github-link flex bg-neutral-100 dark:bg-neutral-800 rounded-lg py-1 px-2 items-center gap-2">
        <Link
          to="https://github.com/janhq/cortex"
          target="_blank"
          className="hover:no-underline text-black dark:text-white hover:text-inherit"
        >
          <div className="font-semibold flex items-center gap-x-0.5">
            <RiStarSFill className="text-yellow-400" />
            <span>{formatCompactNumber(repoInfo.stargazers_count)}</span>
          </div>
        </Link>
      </div>
      <div className="header-discord-link flex bg-neutral-100 dark:bg-neutral-800 rounded-lg py-1 px-2 items-center gap-2">
        <Link
          to="https://discord.gg/FTk2MvZwJH"
          target="_blank"
          className="hover:no-underline text-black dark:text-white hover:text-inherit"
        >
          <div className="font-semibold flex items-center gap-x-0.5">
            <div className="w-2 h-2 bg-green-500 rounded-full" />
            <span>{formatCompactNumber(discordWidget.presence_count)}</span>
          </div>
        </Link>
      </div>
    </div>
  );
};

export default SocialNavbar;
