import Cal, { getCalApi } from "@calcom/embed-react";
  import { useEffect } from "react";
  export default function MyApp() {
	useEffect(()=>{
	  (async function () {
		const cal = await getCalApi({"namespace":"cortex"});
		cal("ui", {"styles":{"branding":{"brandColor":"#000000"}},"hideEventTypeDetails":false,"layout":"month_view"});
	  })();
	}, [])
	return <Cal namespace="cortex"
	  calLink="team/homebrew/cortex"
	  style={{width:"100%",height:"100%",overflow:"scroll"}}
	  config={{"layout":"month_view"}}
    
	  
	/>;
  };
  