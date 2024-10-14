import * as React from "react";

const CardContainer = ({ children }: React.PropsWithChildren) => {
  return <div className="cardContainer">{children}</div>;
};

export default CardContainer;
