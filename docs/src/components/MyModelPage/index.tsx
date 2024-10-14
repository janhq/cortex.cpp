import Layout from "@theme/Layout";
import markdownit from "markdown-it";
import hljs from "highlight.js";

const MyModelPage = (props: { route: any }) => {
  const { route } = props;
  const { customData: data } = route;

  const md = markdownit({
    highlight: function (str: string, lang: string) {
      if (lang && hljs.getLanguage(lang)) {
        try {
          return (
            '<pre><code class="hljs">' +
            hljs.highlight(str, { language: lang, ignoreIllegals: true })
              .value +
            "</code></pre>"
          );
        } catch (__) {}
      }

      return (
        '<pre class="bg-neutral-900 dark:bg-neutral-800 text-neutral-300"><code class="hljs">' +
        md.utils.escapeHtml(str) +
        "</code></pre>"
      );
    },
  });
  const result = md.render(
    data.readmeContent.replace(/---\s*license:\s*([^\s]+)\s*---/m, "")
  );

  return (
    <Layout>
      <div className="container mt-20 models-detail">
        <h1 className="mb-8">{data.name.replace("cortexso/", "")}</h1>
        <div
          dangerouslySetInnerHTML={{
            __html: result,
          }}
        />
      </div>
    </Layout>
  );
};

export default MyModelPage;
