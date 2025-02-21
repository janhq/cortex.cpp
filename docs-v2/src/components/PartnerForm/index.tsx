import React from "react";

import { FilloutStandardEmbed } from "@fillout/react";
import "@fillout/react/style.css";

const PartnerForm = () => {
  return (
    <div className="my-20 h-[1100px]">
      <div
        style={{
          width: "100%",
          height: "100%",
        }}
      >
        <FilloutStandardEmbed filloutId="38SqPFJU2tus" />
      </div>
    </div>
  );
};

export default PartnerForm;


