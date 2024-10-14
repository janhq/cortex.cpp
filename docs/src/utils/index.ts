export function getMonthName(dateString: string) {
  const [month, day, year] = dateString
    .split("-")
    .map((part) => parseInt(part, 10));
  const date = new Date(year, month - 1, day); // Create a Date object
  const monthNames = [
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec",
  ];
  return monthNames[date.getMonth()]; // Get the month name
}

export function formatCompactNumber(count: number) {
  const formatter = Intl.NumberFormat("en", { notation: "compact" });
  return formatter.format(count);
}

export const toGibibytes = (
  input: number,
  options?: { hideUnit?: boolean }
) => {
  if (!input) return "";
  if (input > 1024 ** 3) {
    return (input / 1024 ** 3).toFixed(2) + (options?.hideUnit ? "" : "GB");
  } else if (input > 1024 ** 2) {
    return (input / 1024 ** 2).toFixed(2) + (options?.hideUnit ? "" : "MB");
  } else if (input > 1024) {
    return (input / 1024).toFixed(2) + (options?.hideUnit ? "" : "KB");
  } else {
    return input + (options?.hideUnit ? "" : "B");
  }
};

export function yamlToJSON(yamlString: string): string {
  const lines = yamlString.split("\n");
  const jsonObject: any = {};
  let currentObject: any = jsonObject;
  const stack: any[] = [];

  lines.forEach((line) => {
    line = line.trim();
    if (line.startsWith("#") || line === "") return; // Skip comments and empty lines

    if (line.includes(":")) {
      const [key, value] = line.split(/:(.+)/).map((str) => str.trim());
      if (value === undefined) {
        const newObject: any = {};
        stack.push(currentObject);
        currentObject[key] = newObject;
        currentObject = newObject;
      } else {
        if (value === "" || value === "-") {
          const newObject: any = {};
          stack.push(currentObject);
          currentObject[key] = newObject;
          currentObject = newObject;
        } else {
          currentObject[key] = isNaN(Number(value))
            ? value === "true" || value === "false"
              ? value === "true"
              : value
            : parseFloat(value);
        }
      }
    } else if (line === "-") {
      if (!Array.isArray(currentObject)) {
        const arrayKey = Object.keys(stack[stack.length - 1]).find(
          (key) => stack[stack.length - 1][key] === currentObject
        );
        currentObject = stack[stack.length - 1][arrayKey] = [];
      }
      const newObject: any = {};
      currentObject.push(newObject);
      stack.push(currentObject);
      currentObject = newObject;
    }
  });

  while (stack.length) currentObject = stack.pop(); // Return to top level object

  return JSON.stringify(jsonObject, null, 2);
}
