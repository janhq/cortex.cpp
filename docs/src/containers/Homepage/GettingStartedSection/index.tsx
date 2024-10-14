import Link from "@docusaurus/Link";
import { FaArrowRight } from "react-icons/fa6";

const GettingStartedSection = () => {
  return (
    <>
      <div className="container">
        <div className="m-10 md:mt-40 text-center w-full lg:w-1/2 mx-auto">
          <h2 className="text-5xl font-grotesk">Features</h2>
          <p className="text-black/60 dark:text-white/60 text-lg">
            Cortex allows developers to focus on building local AI applications
            by abstracting hardware and engine support.
          </p>
        </div>

        <div className="flex items-center justify-center mt-8 gap-x-2 cursor-pointer">
          <div className="w-8 h-8 dark:bg-neutral-800 bg-neutral-100 flex rounded-full items-center justify-center">
            <FaArrowRight className="text-blue-300 -rotate-45" size={20} />
          </div>
          <Link to="/docs" className="!no-underline">
            <p className="mb-0 text-neutral-800 font-medium dark:text-white">
              Learn more
            </p>
          </Link>
        </div>
      </div>
    </>
  );
};

export default GettingStartedSection;
