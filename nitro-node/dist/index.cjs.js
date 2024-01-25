'use strict';

var os = require('node:os');
var fs = require('node:fs');
var path = require('node:path');
var node_child_process = require('node:child_process');
var require$$1$2 = require('net');
var require$$1$1 = require('util');
var require$$1 = require('tty');
var require$$0 = require('os');
var require$$1$3 = require('child_process');

function _interopDefaultLegacy (e) { return e && typeof e === 'object' && 'default' in e ? e : { 'default': e }; }

var os__default = /*#__PURE__*/_interopDefaultLegacy(os);
var fs__default = /*#__PURE__*/_interopDefaultLegacy(fs);
var path__default = /*#__PURE__*/_interopDefaultLegacy(path);
var require$$1__default$2 = /*#__PURE__*/_interopDefaultLegacy(require$$1$2);
var require$$1__default$1 = /*#__PURE__*/_interopDefaultLegacy(require$$1$1);
var require$$1__default = /*#__PURE__*/_interopDefaultLegacy(require$$1);
var require$$0__default = /*#__PURE__*/_interopDefaultLegacy(require$$0);
var require$$1__default$3 = /*#__PURE__*/_interopDefaultLegacy(require$$1$3);

/******************************************************************************
Copyright (c) Microsoft Corporation.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THIS SOFTWARE.
***************************************************************************** */

var __assign = function() {
    __assign = Object.assign || function __assign(t) {
        for (var s, i = 1, n = arguments.length; i < n; i++) {
            s = arguments[i];
            for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p)) t[p] = s[p];
        }
        return t;
    };
    return __assign.apply(this, arguments);
};

function __awaiter(thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
}

function __generator(thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (g && (g = 0, op[0] && (_ = 0)), _) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
}

typeof SuppressedError === "function" ? SuppressedError : function (error, suppressed, message) {
    var e = new Error(message);
    return e.name = "SuppressedError", e.error = error, e.suppressed = suppressed, e;
};

function getDefaultExportFromCjs (x) {
	return x && x.__esModule && Object.prototype.hasOwnProperty.call(x, 'default') ? x['default'] : x;
}

var tcpPortUsed = {};

var is2 = {};

var deepIs = {exports: {}};

var pSlice = Array.prototype.slice;
var Object_keys = typeof Object.keys === 'function'
    ? Object.keys
    : function (obj) {
        var keys = [];
        for (var key in obj) keys.push(key);
        return keys;
    }
;

var deepEqual = deepIs.exports = function (actual, expected) {
  // enforce Object.is +0 !== -0
  if (actual === 0 && expected === 0) {
    return areZerosEqual(actual, expected);

  // 7.1. All identical values are equivalent, as determined by ===.
  } else if (actual === expected) {
    return true;

  } else if (actual instanceof Date && expected instanceof Date) {
    return actual.getTime() === expected.getTime();

  } else if (isNumberNaN(actual)) {
    return isNumberNaN(expected);

  // 7.3. Other pairs that do not both pass typeof value == 'object',
  // equivalence is determined by ==.
  } else if (typeof actual != 'object' && typeof expected != 'object') {
    return actual == expected;

  // 7.4. For all other Object pairs, including Array objects, equivalence is
  // determined by having the same number of owned properties (as verified
  // with Object.prototype.hasOwnProperty.call), the same set of keys
  // (although not necessarily the same order), equivalent values for every
  // corresponding key, and an identical 'prototype' property. Note: this
  // accounts for both named and indexed properties on Arrays.
  } else {
    return objEquiv(actual, expected);
  }
};

function isUndefinedOrNull(value) {
  return value === null || value === undefined;
}

function isArguments(object) {
  return Object.prototype.toString.call(object) == '[object Arguments]';
}

function isNumberNaN(value) {
  // NaN === NaN -> false
  return typeof value == 'number' && value !== value;
}

function areZerosEqual(zeroA, zeroB) {
  // (1 / +0|0) -> Infinity, but (1 / -0) -> -Infinity and (Infinity !== -Infinity)
  return (1 / zeroA) === (1 / zeroB);
}

function objEquiv(a, b) {
  if (isUndefinedOrNull(a) || isUndefinedOrNull(b))
    return false;

  // an identical 'prototype' property.
  if (a.prototype !== b.prototype) return false;
  //~~~I've managed to break Object.keys through screwy arguments passing.
  //   Converting to array solves the problem.
  if (isArguments(a)) {
    if (!isArguments(b)) {
      return false;
    }
    a = pSlice.call(a);
    b = pSlice.call(b);
    return deepEqual(a, b);
  }
  try {
    var ka = Object_keys(a),
        kb = Object_keys(b),
        key, i;
  } catch (e) {//happens when one is a string literal and the other isn't
    return false;
  }
  // having the same number of owned properties (keys incorporates
  // hasOwnProperty)
  if (ka.length != kb.length)
    return false;
  //the same set of keys (although not necessarily the same order),
  ka.sort();
  kb.sort();
  //~~~cheap key test
  for (i = ka.length - 1; i >= 0; i--) {
    if (ka[i] != kb[i])
      return false;
  }
  //equivalent values for every corresponding key, and
  //~~~possibly expensive deep test
  for (i = ka.length - 1; i >= 0; i--) {
    key = ka[i];
    if (!deepEqual(a[key], b[key])) return false;
  }
  return true;
}

var deepIsExports = deepIs.exports;

const word = '[a-fA-F\\d:]';
const b = options => options && options.includeBoundaries ?
	`(?:(?<=\\s|^)(?=${word})|(?<=${word})(?=\\s|$))` :
	'';

const v4 = '(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)(?:\\.(?:25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)){3}';

const v6seg = '[a-fA-F\\d]{1,4}';
const v6 = `
(?:
(?:${v6seg}:){7}(?:${v6seg}|:)|                                    // 1:2:3:4:5:6:7::  1:2:3:4:5:6:7:8
(?:${v6seg}:){6}(?:${v4}|:${v6seg}|:)|                             // 1:2:3:4:5:6::    1:2:3:4:5:6::8   1:2:3:4:5:6::8  1:2:3:4:5:6::1.2.3.4
(?:${v6seg}:){5}(?::${v4}|(?::${v6seg}){1,2}|:)|                   // 1:2:3:4:5::      1:2:3:4:5::7:8   1:2:3:4:5::8    1:2:3:4:5::7:1.2.3.4
(?:${v6seg}:){4}(?:(?::${v6seg}){0,1}:${v4}|(?::${v6seg}){1,3}|:)| // 1:2:3:4::        1:2:3:4::6:7:8   1:2:3:4::8      1:2:3:4::6:7:1.2.3.4
(?:${v6seg}:){3}(?:(?::${v6seg}){0,2}:${v4}|(?::${v6seg}){1,4}|:)| // 1:2:3::          1:2:3::5:6:7:8   1:2:3::8        1:2:3::5:6:7:1.2.3.4
(?:${v6seg}:){2}(?:(?::${v6seg}){0,3}:${v4}|(?::${v6seg}){1,5}|:)| // 1:2::            1:2::4:5:6:7:8   1:2::8          1:2::4:5:6:7:1.2.3.4
(?:${v6seg}:){1}(?:(?::${v6seg}){0,4}:${v4}|(?::${v6seg}){1,6}|:)| // 1::              1::3:4:5:6:7:8   1::8            1::3:4:5:6:7:1.2.3.4
(?::(?:(?::${v6seg}){0,5}:${v4}|(?::${v6seg}){1,7}|:))             // ::2:3:4:5:6:7:8  ::2:3:4:5:6:7:8  ::8             ::1.2.3.4
)(?:%[0-9a-zA-Z]{1,})?                                             // %eth0            %1
`.replace(/\s*\/\/.*$/gm, '').replace(/\n/g, '').trim();

// Pre-compile only the exact regexes because adding a global flag make regexes stateful
const v46Exact = new RegExp(`(?:^${v4}$)|(?:^${v6}$)`);
const v4exact = new RegExp(`^${v4}$`);
const v6exact = new RegExp(`^${v6}$`);

const ip = options => options && options.exact ?
	v46Exact :
	new RegExp(`(?:${b(options)}${v4}${b(options)})|(?:${b(options)}${v6}${b(options)})`, 'g');

ip.v4 = options => options && options.exact ? v4exact : new RegExp(`${b(options)}${v4}${b(options)}`, 'g');
ip.v6 = options => options && options.exact ? v6exact : new RegExp(`${b(options)}${v6}${b(options)}`, 'g');

var ipRegex = ip;

var name = "is2";
var version = "2.0.9";
var description = "A type checking library where each exported function returns either true or false and does not throw. Also added tests.";
var license = "MIT";
var tags = [
	"type",
	"check",
	"checker",
	"checking",
	"utilities",
	"network",
	"networking",
	"credit",
	"card",
	"validation"
];
var keywords = [
	"type",
	"check",
	"checker",
	"checking",
	"utilities",
	"network",
	"networking",
	"credit",
	"card",
	"validation"
];
var author = "Enrico Marino <enrico.marino@email.com>";
var maintainers = "Edmond Meinfelder <edmond@stdarg.com>, Chris Oyler <christopher.oyler@gmail.com>";
var homepage = "http://github.com/stdarg/is2";
var repository = {
	type: "git",
	url: "git@github.com:stdarg/is2.git"
};
var bugs = {
	url: "http://github.com/stdarg/is/issues"
};
var main = "./index.js";
var scripts = {
	test: "./node_modules/.bin/mocha -C --reporter list tests.js"
};
var engines = {
	node: ">=v0.10.0"
};
var dependencies = {
	"deep-is": "^0.1.3",
	"ip-regex": "^4.1.0",
	"is-url": "^1.2.4"
};
var devDependencies = {
	mocha: "6.2.3",
	mongodb: "3.2.4"
};
var require$$2 = {
	name: name,
	version: version,
	description: description,
	license: license,
	tags: tags,
	keywords: keywords,
	author: author,
	maintainers: maintainers,
	homepage: homepage,
	repository: repository,
	bugs: bugs,
	main: main,
	scripts: scripts,
	engines: engines,
	dependencies: dependencies,
	devDependencies: devDependencies
};

var isUrl_1;
var hasRequiredIsUrl;

function requireIsUrl () {
	if (hasRequiredIsUrl) return isUrl_1;
	hasRequiredIsUrl = 1;
	/**
	 * Expose `isUrl`.
	 */

	isUrl_1 = isUrl;

	/**
	 * RegExps.
	 * A URL must match #1 and then at least one of #2/#3.
	 * Use two levels of REs to avoid REDOS.
	 */

	var protocolAndDomainRE = /^(?:\w+:)?\/\/(\S+)$/;

	var localhostDomainRE = /^localhost[\:?\d]*(?:[^\:?\d]\S*)?$/;
	var nonLocalhostDomainRE = /^[^\s\.]+\.\S{2,}$/;

	/**
	 * Loosely validate a URL `string`.
	 *
	 * @param {String} string
	 * @return {Boolean}
	 */

	function isUrl(string){
	  if (typeof string !== 'string') {
	    return false;
	  }

	  var match = string.match(protocolAndDomainRE);
	  if (!match) {
	    return false;
	  }

	  var everythingAfterProtocol = match[1];
	  if (!everythingAfterProtocol) {
	    return false;
	  }

	  if (localhostDomainRE.test(everythingAfterProtocol) ||
	      nonLocalhostDomainRE.test(everythingAfterProtocol)) {
	    return true;
	  }

	  return false;
	}
	return isUrl_1;
}

/**
 * @fileOverview
 * is2 derived from is by Enrico Marino, adapted for Node.js.
 * Slightly modified by Edmond Meinfelder
 *
 * is
 * the definitive JavaScript type testing library
 * Copyright(c) 2013,2014 Edmond Meinfelder <edmond@stdarg.com>
 * Copyright(c) 2011 Enrico Marino <enrico.marino@email.com>
 * MIT license
 */

(function (exports) {
	const owns = {}.hasOwnProperty;
	const toString = {}.toString;
	const is = exports;
	const deepIs = deepIsExports;
	const ipRegEx =  ipRegex;
	is.version = require$$2.version;

	////////////////////////////////////////////////////////////////////////////////
	// Environment

	/**
	 * Tests if is is running under a browser.
	 * @return {Boolean} true if the environment has process, process.version and process.versions.
	 */
	is.browser = function() {
	    return (!is.node() && typeof window !== 'undefined' && toString.call(window) === '[object global]');
	};

	/**
	 * Test if 'value' is defined.
	 * Alias: def
	 * @param {Any} value The value to test.
	 * @return {Boolean} true if 'value' is defined, false otherwise.
	 */
	is.defined = function(value) {
	    return typeof value !== 'undefined';
	};
	is.def = is.defined;

	/**
	 * Tests if is is running under node.js
	 * @return {Boolean} true if the environment has process, process.version and process.versions.
	 */
	is.nodejs = function() {
	    return (process && process.hasOwnProperty('version') &&
	            process.hasOwnProperty('versions'));
	};
	is.node = is.nodejs;

	/**
	 * Test if 'value' is undefined.
	 * Aliases: undef, udef
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is undefined, false otherwise.
	 */
	is.undefined = function(value) {
	    return value === undefined;
	};
	is.udef = is.undef = is.undefined;


	////////////////////////////////////////////////////////////////////////////////
	// Types

	/**
	 * Test if 'value' is an array.
	 * Alias: ary, arry
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is an array, false otherwise.
	 */
	is.array = function(value) {
	    return '[object Array]' === toString.call(value);
	};
	is.arr = is.ary = is.arry = is.array;

	/**
	 * Test if 'value' is an arraylike object (i.e. it has a length property with a valid value)
	 * Aliases: arraylike, arryLike, aryLike
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is an arguments object, false otherwise.
	 */
	is.arrayLike = function(value) {
	    if (is.nullOrUndef(value))
	        return false;
	    return value !== undefined &&
	        owns.call(value, 'length') &&
	        isFinite(value.length);
	};
	is.arrLike = is.arryLike = is.aryLike = is.arraylike = is.arrayLike;

	/**
	 * Test if 'value' is an arguments object.
	 * Alias: args
	 * @param {Any} value value to test
	 * @return {Boolean} true if 'value' is an arguments object, false otherwise
	 */
	is.arguments = function(value) {
	    return '[object Arguments]' === toString.call(value);
	};
	is.args = is.arguments;

	/**
	 * Test if 'value' is a boolean.
	 * Alias: bool
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a boolean, false otherwise.
	 */
	is.boolean = function(value) {
	    return '[object Boolean]' === toString.call(value);
	};
	is.bool = is.boolean;

	/**
	 * Test if 'value' is an instance of Buffer.
	 * Aliases: instOf, instanceof
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is an instance of 'constructor'.
	 */
	is.buffer = function(value) {
	    return is.nodejs() && Buffer && Buffer.hasOwnProperty('isBuffer') && Buffer.isBuffer(value);
	};
	is.buff = is.buf = is.buffer;

	/**
	 * Test if 'value' is a date.
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a date, false otherwise.
	 */
	is.date = function(value) {
	    return '[object Date]' === toString.call(value);
	};

	/**
	 * Test if 'value' is an error object.
	 * Alias: err
	 * @param value value to test.
	 * @return {Boolean} true if 'value' is an error object, false otherwise.
	 */
	is.error = function(value) {
	    return '[object Error]' === toString.call(value);
	};
	is.err = is.error;

	/**
	 * Test if 'value' is false.
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is false, false otherwise
	 */
	is.false = function(value) {
	    return value === false;
	};

	/**
	 * Test if 'value' is a function or async function.
	 * Alias: func
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a function, false otherwise.
	 */
	is.function = function(value) {
	    return is.syncFunction(value) || is.asyncFunction(value)
	};
	is.fun = is.func = is.function;

	/**
	 * Test if 'value' is an async function using `async () => {}` or `async function () {}`.
	 * Alias: func
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a function, false otherwise.
	 */
	is.asyncFunction = function(value) {
	  return '[object AsyncFunction]' === toString.call(value);
	};
	is.asyncFun = is.asyncFunc = is.asyncFunction;

	/**
	 * Test if 'value' is a synchronous function.
	 * Alias: syncFunc
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a function, false otherwise.
	 */
	is.syncFunction = function (value) {
	  return '[object Function]' === toString.call(value);
	};
	is.syncFun = is.syncFunc = is.syncFunction;
	/**
	 * Test if 'value' is null.
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is null, false otherwise.
	 */
	is.null = function(value) {
	    return value === null;
	};

	/**
	 * Test is 'value' is either null or undefined.
	 * Alias: nullOrUndef
	 * @param {Any} value value to test.
	 * @return {Boolean} True if value is null or undefined, false otherwise.
	 */
	is.nullOrUndefined = function(value) {
	    return value === null || typeof value === 'undefined';
	};
	is.nullOrUndef = is.nullOrUndefined;

	/**
	 * Test if 'value' is a number.
	 * Alias: num
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a number, false otherwise.
	 */
	is.number = function(value) {
	    return '[object Number]' === toString.call(value);
	};
	is.num = is.number;

	/**
	 * Test if 'value' is an object. Note: Arrays, RegExps, Date, Error, etc all return false.
	 * Alias: obj
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is an object, false otherwise.
	 */
	is.object = function(value) {
	    return '[object Object]' === toString.call(value);
	};
	is.obj = is.object;

	/**
	 * Test if 'value' is a regular expression.
	 * Alias: regexp
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a regexp, false otherwise.
	 */
	is.regExp = function(value) {
	    return '[object RegExp]' === toString.call(value);
	};
	is.re = is.regexp = is.regExp;

	/**
	 * Test if 'value' is a string.
	 * Alias: str
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a string, false otherwise.
	 */
	is.string = function(value) {
	    return '[object String]' === toString.call(value);
	};
	is.str = is.string;

	/**
	 * Test if 'value' is true.
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is true, false otherwise.
	 */
	is.true = function(value) {
	    return value === true;
	};

	/**
	 * Test if 'value' is a uuid (v1-v5)
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value is a valid RFC4122 UUID. Case non-specific.
	 */
	var uuidRegExp = new RegExp('[0-9a-f]{8}-[0-9a-f]{4}-[1-5][0-9a-f]{3}-[89ab]'+
	                            '[0-9a-f]{3}-[0-9a-f]{12}', 'i');
	is.uuid = function(value) {
	    return uuidRegExp.test(value);
	};

	////////////////////////////////////////////////////////////////////////////////
	// Object Relationships

	/**
	 * Test if 'value' is equal to 'other'. Works for objects and arrays and will do deep comparisions,
	 * using recursion.
	 * Alias: eq
	 * @param {Any} value value.
	 * @param {Any} other value to compare with.
	 * @return {Boolean} true if 'value' is equal to 'other', false otherwise
	 */
	is.equal = function(value, other) {
	    var type = toString.call(value);

	    if (typeof value !== typeof other) {
	        return false;
	    }

	    if (type !== toString.call(other)) {
	        return false;
	    }

	    if ('[object Object]' === type || '[object Array]' === type) {
	        return deepIs(value, other);
	    } else if ('[object Function]' === type) {
	        return value.prototype === other.prototype;
	    } else if ('[object Date]' === type) {
	        return value.getTime() === other.getTime();
	    }

	    return value === other;
	};
	is.objEquals = is.eq = is.equal;

	/**
	 * JS Type definitions which cannot host values.
	 * @api private
	 */
	var NON_HOST_TYPES = {
	    'boolean': 1,
	    'number': 1,
	    'string': 1,
	    'undefined': 1
	};

	/**
	 * Test if 'key' in host is an object. To be hosted means host[value] is an object.
	 * @param {Any} value The value to test.
	 * @param {Any} host Host that may contain value.
	 * @return {Boolean} true if 'value' is hosted by 'host', false otherwise.
	 */
	is.hosted = function(value, host) {
	    if (is.nullOrUndef(value))
	        return false;
	    var type = typeof host[value];
	    return type === 'object' ? !!host[value] : !NON_HOST_TYPES[type];
	};

	/**
	 * Test if 'value' is an instance of 'constructor'.
	 * Aliases: instOf, instanceof
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is an instance of 'constructor'.
	 */
	is.instanceOf = function(value, constructor) {
	    if (is.nullOrUndef(value) || is.nullOrUndef(constructor))
	        return false;
	    return (value instanceof constructor);
	};
	is.instOf = is.instanceof = is.instanceOf;

	/**
	 * Test if 'value' is an instance type objType.
	 * Aliases: objInstOf, objectinstanceof, instOf, instanceOf
	 * @param {object} objInst an object to testfor type.
	 * @param {object} objType an object type to compare.
	 * @return {Boolean} true if 'value' is an object, false otherwise.
	 */
	is.objectInstanceOf = function(objInst, objType) {
	    try {
	        return '[object Object]' === toString.call(objInst) && (objInst instanceof objType);
	    } catch(err) {
	        return false;
	    }
	};
	is.instOf = is.instanceOf = is.objInstOf = is.objectInstanceOf;

	/**
	 * Test if 'value' is a type of 'type'.
	 * Alias: a
	 * @param value value to test.
	 * @param {String} type The name of the type.
	 * @return {Boolean} true if 'value' is an arguments object, false otherwise.
	 */
	is.type = function(value, type) {
	    return typeof value === type;
	};
	is.a = is.type;

	////////////////////////////////////////////////////////////////////////////////
	// Object State

	/**
	 * Test if 'value' is empty. To be empty means to be an array, object or string with nothing contained.
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is empty, false otherwise.
	 */
	is.empty = function(value) {
	    var type = toString.call(value);

	    if ('[object Array]' === type || '[object Arguments]' === type) {
	        return value.length === 0;
	    }

	    if ('[object Object]' === type) {
	        for (var key in value) if (owns.call(value, key)) return false;
	        return true;
	    }

	    if ('[object String]' === type) {
	        return value === '';
	    }

	    return false;
	};

	/**
	 * Test if 'value' is an arguments object that is empty.
	 * Alias: args
	 * @param {Any} value value to test
	 * @return {Boolean} true if 'value' is an arguments object with no args, false otherwise
	 */
	is.emptyArguments = function(value) {
	    return '[object Arguments]' === toString.call(value) && value.length === 0;
	};
	is.noArgs = is.emptyArgs = is.emptyArguments;

	/**
	 * Test if 'value' is an array containing no entries.
	 * Aliases: emptyArry, emptyAry
	 * @param {Any} value The value to test.
	 * @return {Boolean} true if 'value' is an array with no elemnets.
	 */
	is.emptyArray = function(value) {
	    return '[object Array]' === toString.call(value) && value.length === 0;
	};
	is.emptyArry = is.emptyAry = is.emptyArray;

	/**
	 * Test if 'value' is an empty array(like) object.
	 * Aliases: arguents.empty, args.empty, ary.empty, arry.empty
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is an empty array(like), false otherwise.
	 */
	is.emptyArrayLike = function(value) {
	    return value.length === 0;
	};
	is.emptyArrLike = is.emptyArrayLike;

	/**
	 * Test if 'value' is an empty string.
	 * Alias: emptyStr
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is am empty string, false otherwise.
	 */
	is.emptyString = function(value) {
	    return is.string(value) && value.length === 0;
	};
	is.emptyStr = is.emptyString;

	/**
	 * Test if 'value' is an array containing at least 1 entry.
	 * Aliases: nonEmptyArry, nonEmptyAry
	 * @param {Any} value The value to test.
	 * @return {Boolean} true if 'value' is an array with at least 1 value, false otherwise.
	 */
	is.nonEmptyArray = function(value) {
	    return '[object Array]' === toString.call(value) && value.length > 0;
	};
	is.nonEmptyArr = is.nonEmptyArry = is.nonEmptyAry = is.nonEmptyArray;

	/**
	 * Test if 'value' is an object with properties. Note: Arrays are objects.
	 * Alias: nonEmptyObj
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is an object, false otherwise.
	 */
	is.nonEmptyObject = function(value) {
	    return '[object Object]' === toString.call(value) && Object.keys(value).length > 0;
	};
	is.nonEmptyObj = is.nonEmptyObject;

	/**
	 * Test if 'value' is an object with no properties. Note: Arrays are objects.
	 * Alias: nonEmptyObj
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is an object, false otherwise.
	 */
	is.emptyObject = function(value) {
	    return '[object Object]' === toString.call(value) && Object.keys(value).length === 0;
	};
	is.emptyObj = is.emptyObject;

	/**
	 * Test if 'value' is a non-empty string.
	 * Alias: nonEmptyStr
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a non-empty string, false otherwise.
	 */
	is.nonEmptyString = function(value) {
	    return is.string(value) && value.length > 0;
	};
	is.nonEmptyStr = is.nonEmptyString;

	////////////////////////////////////////////////////////////////////////////////
	// Numeric Types within Number

	/**
	 * Test if 'value' is an even number.
	 * @param {Number} value to test.
	 * @return {Boolean} true if 'value' is an even number, false otherwise.
	 */
	is.even = function(value) {
	    return '[object Number]' === toString.call(value) && value % 2 === 0;
	};

	/**
	 * Test if 'value' is a decimal number.
	 * Aliases: decimalNumber, decNum
	 * @param {Any} value value to test.
	 * @return {Boolean} true if 'value' is a decimal number, false otherwise.
	 */
	is.decimal = function(value) {
	    return '[object Number]' === toString.call(value) && value % 1 !== 0;
	};
	is.dec = is.decNum = is.decimal;

	/**
	 * Test if 'value' is an integer.
	 * Alias: integer
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is an integer, false otherwise.
	 */
	is.integer = function(value) {
	    return '[object Number]' === toString.call(value) && value % 1 === 0;
	};
	is.int = is.integer;

	/**
	 * is.nan
	 * Test if `value` is not a number.
	 *
	 * @param {Mixed} value value to test
	 * @return {Boolean} true if `value` is not a number, false otherwise
	 * @api public
	 */
	is.notANumber = function(value) {
	    return !is.num(value) || value !== value;
	};
	is.nan = is.notANum = is.notANumber;

	/**
	 * Test if 'value' is an odd number.
	 * @param {Number} value to test.
	 * @return {Boolean} true if 'value' is an odd number, false otherwise.
	 */
	is.odd = function(value) {
	    return !is.decimal(value) && '[object Number]' === toString.call(value) && value % 2 !== 0;
	};
	is.oddNumber = is.oddNum = is.odd;

	////////////////////////////////////////////////////////////////////////////////
	// Numeric Type & State

	/**
	 * Test if 'value' is a positive number.
	 * Alias: positiveNum, posNum
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a number, false otherwise.
	 */
	is.positiveNumber = function(value) {
	    return '[object Number]' === toString.call(value) && value > 0;
	};
	is.pos = is.positive = is.posNum = is.positiveNum = is.positiveNumber;

	/**
	 * Test if 'value' is a negative number.
	 * Aliases: negNum, negativeNum
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a number, false otherwise.
	 */
	is.negativeNumber = function(value) {
	    return '[object Number]' === toString.call(value) && value < 0;
	};
	is.neg = is.negNum = is.negativeNum = is.negativeNumber;

	/**
	 * Test if 'value' is a negative integer.
	 * Aliases: negInt, negativeInteger
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a negative integer, false otherwise.
	 */
	is.negativeInteger = function(value) {
	    return '[object Number]' === toString.call(value) && value % 1 === 0 && value < 0;
	};
	is.negativeInt = is.negInt = is.negativeInteger;

	/**
	 * Test if 'value' is a positive integer.
	 * Alias: posInt
	 * @param {Any} value to test.
	 * @return {Boolean} true if 'value' is a positive integer, false otherwise.
	 */
	is.positiveInteger = function(value) {
	    return '[object Number]' === toString.call(value) && value % 1 === 0 && value > 0;
	};
	is.posInt = is.positiveInt = is.positiveInteger;

	////////////////////////////////////////////////////////////////////////////////
	// Numeric Relationships

	/**
	 * Test if 'value' is divisible by 'n'.
	 * Alias: divisBy
	 * @param {Number} value value to test.
	 * @param {Number} n dividend.
	 * @return {Boolean} true if 'value' is divisible by 'n', false otherwise.
	 */
	is.divisibleBy = function(value, n) {
	    if (value === 0)
	        return false;
	    return '[object Number]' === toString.call(value) &&
	        n !== 0 &&
	        value % n === 0;
	};
	is.divBy = is.divisBy = is.divisibleBy;

	/**
	 * Test if 'value' is greater than or equal to 'other'.
	 * Aliases: greaterOrEq, greaterOrEqual
	 * @param {Number} value value to test.
	 * @param {Number} other value to compare with.
	 * @return {Boolean} true, if value is greater than or equal to other, false otherwise.
	 */
	is.greaterOrEqualTo = function(value, other) {
	    return value >= other;
	};
	is.greaterOrEqual = is.ge = is.greaterOrEqualTo;

	/**
	 * Test if 'value' is greater than 'other'.
	 * Aliases: greaterThan
	 * @param {Number} value value to test.
	 * @param {Number} other value to compare with.
	 * @return {Boolean} true, if value is greater than other, false otherwise.
	 */
	is.greaterThan = function(value, other) {
	    return value > other;
	};
	is.gt = is.greaterThan;

	/**
	 * Test if 'value' is less than or equal to 'other'.
	 * Alias: lessThanOrEq, lessThanOrEqual
	 * @param {Number} value value to test
	 * @param {Number} other value to compare with
	 * @return {Boolean} true, if 'value' is less than or equal to 'other', false otherwise.
	 */
	is.lessThanOrEqualTo = function(value, other) {
	    return value <= other;
	};
	is.lessThanOrEq = is.lessThanOrEqual = is.le = is.lessThanOrEqualTo;

	/**
	 * Test if 'value' is less than 'other'.
	 * Alias: lessThan
	 * @param {Number} value value to test
	 * @param {Number} other value to compare with
	 * @return {Boolean} true, if 'value' is less than 'other', false otherwise.
	 */
	is.lessThan = function(value, other) {
	    return value < other;
	};
	is.lt = is.lessThan;

	/**
	 * Test if 'value' is greater than 'others' values.
	 * Alias: max
	 * @param {Number} value value to test.
	 * @param {Array} others values to compare with.
	 * @return {Boolean} true if 'value' is greater than 'others' values.
	 */
	is.maximum = function(value, others) {
	    if (!is.arrayLike(others) || !is.number(value))
	        return false;

	    var len = others.length;
	    while (--len > -1) {
	        if (value < others[len]) {
	            return false;
	        }
	    }

	    return true;
	};
	is.max = is.maximum;

	/**
	 * Test if 'value' is less than 'others' values.
	 * Alias: min
	 * @param {Number} value value to test.
	 * @param {Array} others values to compare with.
	 * @return {Boolean} true if 'value' is less than 'others' values.
	 */
	is.minimum = function(value, others) {
	    if (!is.arrayLike(others) || !is.number(value))
	        return false;

	    var len = others.length;
	    while (--len > -1) {
	        if (value > others[len]) {
	            return false;
	        }
	    }

	    return true;
	};
	is.min = is.minimum;

	/**
	 * Test if 'value' is within 'start' and 'finish'.
	 * Alias: withIn
	 * @param {Number} value value to test.
	 * @param {Number} start lower bound.
	 * @param {Number} finish upper bound.
	 * @return {Boolean} true if 'value' is is within 'start' and 'finish', false otherwise.
	 */
	is.within = function(value, start, finish) {
	    return value >= start && value <= finish;
	};
	is.withIn = is.within;

	/**
	 * Test if 'value' is within 'precision' decimal places from 'comparitor'.
	 * Alias: closish, near.
	 * @param {Number} value value to test
	 * @param {Number} comparitor value to test 'value' against
	 * @param {Number} precision number of decimals to compare floating points, defaults to 2
	 * @return {Boolean} true if 'value' is within 'precision' decimal places from 'comparitor', false otherwise.
	 */
	is.prettyClose = function(value, comparitor, precision) {
	  if (!is.number(value) || !is.number(comparitor)) return false;
	  if (is.defined(precision) && !is.posInt(precision)) return false;
	  if (is.undefined(precision)) precision = 2;

	  return value.toFixed(precision) === comparitor.toFixed(precision);
	};
	is.closish = is.near = is.prettyClose;
	////////////////////////////////////////////////////////////////////////////////
	// Networking

	/**
	 * Test if a value is a valid DNS address. eg www.stdarg.com is true while
	 * 127.0.0.1 is false.
	 * @param {Any} value to test if a DNS address.
	 * @return {Boolean} true if a DNS address, false otherwise.
	 * DNS Address is made up of labels separated by '.'
	 * Each label must be between 1 and 63 characters long
	 * The entire hostname (including the delimiting dots) has a maximum of 255 characters.
	 * Hostname may not contain other characters, such as the underscore character (_)
	 * other DNS names may contain the underscore.
	 */
	is.dnsAddress = function(value) {
	    if (!is.nonEmptyStr(value))  return false;
	    if (value.length > 255)  return false;
	    if (numbersLabel.test(value))  return false;
	    if (!dnsLabel.test(value))  return false;
	    return true;
	    //var names = value.split('.');
	    //if (!is.array(names) || !names.length)  return false;
	    //if (names[0].indexOf('_') > -1)  return false;
	    //for (var i=0; i<names.length; i++) {
	        //if (!dnsLabel.test(names[i]))  return false;
	    //}
	    //return true;
	};
	is.dnsAddr = is.dns = is.dnsAddress;
	var dnsLabel = /^([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])(\.([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]{0,61}[a-zA-Z0-9]))*$/;
	var numbersLabel = /^([0-9]|[0-9][0-9\-]{0,61}[0-9])(\.([0-9]|[0-9][0-9\-]{0,61}[0-9]))*$/;

	/**
	 * Test if value is a valid email address.
	 * @param {Any} value to test if an email address.
	 * @return {Boolean} true if an email address, false otherwise.
	 */
	is.emailAddress = function(value) {
	    if (!is.nonEmptyStr(value))
	        return false;
	    return emailRegexp.test(value);
	};
	is.email = is.emailAddr = is.emailAddress;
	var emailRegexp = /^([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x22([^\x0d\x22\x5c\x80-\xff]|\x5c[\x00-\x7f])*\x22)(\x2e([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x22([^\x0d\x22\x5c\x80-\xff]|\x5c[\x00-\x7f])*\x22))*\x40([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x5b([^\x0d\x5b-\x5d\x80-\xff]|\x5c[\x00-\x7f])*\x5d)(\x2e([^\x00-\x20\x22\x28\x29\x2c\x2e\x3a-\x3c\x3e\x40\x5b-\x5d\x7f-\xff]+|\x5b([^\x0d\x5b-\x5d\x80-\xff]|\x5c[\x00-\x7f])*\x5d))*$/;

	/**
	 * Test if a value is either an IPv4 numeric IP address.
	 * The rules are:
	 * must be a string
	 * length must be 15 characters or less
	 * There must be four octets separated by a '.'
	 * No octet can be less than 0 or greater than 255.
	 * @param {Any} value to test if an ip address.
	 * @return {Boolean} true if an ip address, false otherwise.
	 */
	is.ipv4Address = function(value) {
	    if (!is.nonEmptyStr(value))  return false;
	    if (value.length > 15)  return false;
	    var octets = value.split('.');
	    if (!is.array(octets) || octets.length !== 4)  return false;
	    for (var i=0; i<octets.length; i++) {
	        var val = parseInt(octets[i], 10);
	        if (isNaN(val))  return false;
	        if (val < 0 || val > 255)  return false;
	    }
	    return true;
	};
	is.ipv4 = is.ipv4Addr = is.ipv4Address;

	/**
	 * Test if a value is either an IPv6 numeric IP address.
	 * @param {Any} value to test if an ip address.
	 * @return {Boolean} true if an ip address, false otherwise.
	 */
	is.ipv6Address = function(value) {
	    if (!is.nonEmptyStr(value))  return false;
	    return ipRegEx.v6({extract: true}).test(value);
	};
	is.ipv6 = is.ipv6Addr = is.ipv6Address;

	/**
	 * Test if a value is either an IPv4 or IPv6 numeric IP address.
	 * @param {Any} value to test if an ip address.
	 * @return {Boolean} true if an ip address, false otherwise.
	 */
	is.ipAddress = function(value) {
	    if (!is.nonEmptyStr(value)) return false;
	    return is.ipv4Address(value) || is.ipv6Address(value)
	};
	is.ip = is.ipAddr = is.ipAddress;

	/**
	 * Test is a value is a valid ipv4, ipv6 or DNS name.
	 * Aliases: host, hostAddr, hostAddress.
	 * @param {Any} value to test if a host address.
	 * @return {Boolean} true if a host address, false otherwise.
	 */
	is.hostAddress = function(value) {
	    if (!is.nonEmptyStr(value)) return false;
	    return is.dns(value) || is.ipv4(value) || is.ipv6(value);
	};
	is.host = is.hostIp = is.hostAddr = is.hostAddress;

	/**
	 * Test if a number is a valid TCP port
	 * @param {Any} value to test if its a valid TCP port
	 */
	is.port = function(value) {
	    if (!is.num(value) || is.negativeInt(value) || value > 65535)
	        return false;
	    return true;
	};

	/**
	 * Test if a number is a valid TCP port in the range 0-1023.
	 * Alias: is.sysPort.
	 * @param {Any} value to test if its a valid TCP port
	 */
	is.systemPort = function(value) {
	    if (is.port(value) && value < 1024)
	        return true;
	    return false;
	};
	is.sysPort = is.systemPort;

	/**
	 * Test if a number is a valid TCP port in the range 1024-65535.
	 * @param {Any} value to test if its a valid TCP port
	 */
	is.userPort = function(value) {
	    if (is.port(value) && value > 1023)
	        return true;
	    return false;
	};

	/*
	function sumDigits(num) {
	    var str = num.toString();
	    var sum = 0;
	    for (var i = 0; i < str.length; i++)
	        sum += (str[i]-0);
	    return sum;
	}
	*/

	/**
	 * Test if a string is a credit card.
	 * From http://en.wikipedia.org/wiki/Luhn_algorithm
	 * @param {String} value to test if a credit card.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.creditCardNumber = function(str) {
	    if (!is.str(str))
	        return false;

	    var ary = str.split('');
	    var i, cnt;
	    // From the rightmost digit, which is the check digit, moving left, double
	    // the value of every second digit;
	    for (i=ary.length-1, cnt=1; i>-1; i--, cnt++) {
	        if (cnt%2 === 0)
	            ary[i] *= 2;
	    }

	    str = ary.join('');
	    var sum = 0;
	    // if the product of the previous doubling operation is greater than 9
	    // (e.g., 7 * 2 = 14), then sum the digits of the products (e.g., 10: 1 + 0
	    // = 1, 14: 1 + 4 = 5).  We do the this by joining the array of numbers and
	    // add adding the int value of all the characters in the string.
	    for (i=0; i<str.length; i++)
	        sum += Math.floor(str[i]);

	    // If the total (sum) modulo 10 is equal to 0 (if the total ends in zero)
	    // then the number is valid according to the Luhn formula; else it is not
	    // valid.
	    return sum % 10 === 0;
	};
	is.creditCard = is.creditCardNum = is.creditCardNumber;


	////////////////////////////////////////////////////////////////////////////////
	// The following credit card info is from:
	// http://en.wikipedia.org/wiki/Bank_card_number#Issuer_identification_number_.28IIN.29

	/**
	 * Test if card number is an American Express card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.americanExpressCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 15)
	        return false;

	    var prefix = Math.floor(str.slice(0,2));
	    if (prefix !== 34 && prefix !== 37)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.amexCard = is.amexCardNum = is.americanExpressCardNumber;

	/**
	 * Test if card number is a China UnionPay card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.chinaUnionPayCardNumber = function(str) {
	    if (!is.str(str) || (str.length < 16 && str.length > 19))
	        return false;

	    var prefix = Math.floor(str.slice(0,2));
	    if (prefix !== 62 && prefix !== 88)
	        return false;

	    // no validation for this card
	    return true;
	};
	is.chinaUnion = is.chinaUnionPayCard = is.chinaUnionPayCardNumber;

	/**
	 * Test if card number is a Diner's Club Carte Blance card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.dinersClubCarteBlancheCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 14)
	        return false;

	    var prefix = Math.floor(str.slice(0,3));
	    if (prefix < 300 || prefix > 305)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.dinersClubCB = is.dinersClubCarteBlancheCard =
	    is.dinersClubCarteBlancheCardNumber;

	/**
	 * Test if card number is a Diner's Club International card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.dinersClubInternationalCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 14)
	        return false;
	    var prefix = Math.floor(str.slice(0,3));
	    var prefix2 = Math.floor(str.slice(0,2));

	    // 300-305, 309, 36, 38-39
	    if ((prefix < 300 || prefix > 305) && prefix !== 309 && prefix2 !== 36 &&
	        (prefix2 < 38 || prefix2 > 39)) {
	        return false;
	    }

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.dinersClubInt = is.dinersClubInternationalCard =
	    is.dinersClubInternationalCardNumber;

	/**
	 * Test if card number is a Diner's Club USA & CA card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.dinersClubUSACanadaCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;
	    var prefix = Math.floor(str.slice(0,2));

	    if (prefix !== 54 && prefix !== 55)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.dinersClub = is.dinersClubUSACanCard = is.dinersClubUSACanadaCardNumber;

	/**
	 * Test if card number is a Diner's Club USA/CA card.
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.discoverCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    var prefix = Math.floor(str.slice(0,6));
	    var prefix2 = Math.floor(str.slice(0,3));

	    if (str.slice(0,4) !== '6011' && (prefix < 622126 || prefix > 622925) &&
	        (prefix2 < 644 || prefix2 > 649) && str.slice(0,2) !== '65') {
	        return false;
	    }

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.discover = is.discoverCard = is.discoverCardNumber;

	/**
	 * Test if card number is an InstaPayment card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.instaPaymentCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    var prefix = Math.floor(str.slice(0,3));
	    if (prefix < 637 || prefix > 639)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.instaPayment = is.instaPaymentCardNumber;

	/**
	 * Test if card number is a JCB card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.jcbCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    var prefix = Math.floor(str.slice(0,4));
	    if (prefix < 3528 || prefix > 3589)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.jcb = is.jcbCard = is.jcbCardNumber;

	/**
	 * Test if card number is a Laser card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.laserCardNumber = function(str) {
	    if (!is.str(str) || (str.length < 16 && str.length > 19))
	        return false;

	    var prefix = Math.floor(str.slice(0,4));
	    var valid = [ 6304, 6706, 6771, 6709 ];
	    if (valid.indexOf(prefix) === -1)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.laser = is.laserCard = is.laserCardNumber;

	/**
	 * Test if card number is a Maestro card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.maestroCardNumber = function(str) {
	    if (!is.str(str) || str.length < 12 || str.length > 19)
	        return false;

	    var prefix = str.slice(0,4);
	    var valid = [ '5018', '5020', '5038', '5612', '5893', '6304', '6759',
	        '6761', '6762', '6763', '0604', '6390' ];

	    if (valid.indexOf(prefix) === -1)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.maestro = is.maestroCard = is.maestroCardNumber;

	/**
	 * Test if card number is a Dankort card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.dankortCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    if (str.slice(0,4) !== '5019')
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.dankort = is.dankortCard = is.dankortCardNumber;

	/**
	 * Test if card number is a MasterCard card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.masterCardCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    var prefix = Math.floor(str.slice(0,2));
	    if (prefix < 50 || prefix > 55)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};
	is.masterCard = is.masterCardCard = is.masterCardCardNumber;

	/**
	 * Test if card number is a Visa card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.visaCardNumber = function(str) {
	    if (!is.str(str) || (str.length !== 13 && str.length !== 16))
	        return false;

	    if ('4' !== str.slice(0,1))
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return true;
	};

	is.visa = is.visaCard = is.visaCardNumber;

	/**
	 * Test if card number is a Visa card number
	 * @param {String} the credit card number string to test.
	 * @return true if the string is the correct format, false otherwise
	 */
	is.visaElectronCardNumber = function(str) {
	    if (!is.str(str) || str.length !== 16)
	        return false;

	    var prefix = Math.floor(str.slice(0,4));
	    var valid = [ 4026, 4405, 4508, 4844, 4913, 4917 ];
	    if ('417500' !== str.slice(0,6) && valid.indexOf(prefix) === -1)
	        return false;

	    if (!is.creditCardNumber(str))
	        return false;

	    return false;
	};

	is.visaElectron = is.visaElectronCard = is.visaElectronCardNumber;

	/**
	 * Test if the input is a valid MongoDB id.
	 * @param {String|Object} Either a mongodb object id or a string representation.
	 * @return true if the string is the correct format, false otherwise
	 * Thanks to Jason Denizac (https://github.com/jden) for pointing this out.
	 * https://github.com/jden/objectid/blob/master/index.js#L7-L10
	 */
	var objIdPattern = /^[0-9a-fA-F]{24}$/;
	is.mongoId = is.objectId = is.objId = function(id) {
	  return (Boolean(id) && !Array.isArray(id) && objIdPattern.test(String(id)));
	};

	/**
	 * Test is the first argument is structly equal to any of the subsequent args.
	 * @param Value to test against subsequent arguments.
	 * @return true if the first value matches any of subsequent values.
	 */
	is.matching = is.match = is.inArgs = function(val) {
	    if (arguments.length < 2)
	        return false;
	    var result = false;
	    for (var i=1; i<arguments.length; i++) {
	        var eq = is.equal(val, arguments[i]);
	        result = result || eq;
	    }
	    return result;
	};



	// US Address components
	/**********************************
	***Definitely a work in progress***
	**********************************/
	/**
	 * Test if a string contains a US street address
	 * @param {String} the string to search
	 * @return true if an address is present, false otherwise
	 */
	is.streetAddress = function(str) {
	  if (!is.str(str))
	      return false;

	  var regex = /\b\d+[\s](?:[A-Za-z0-9.-]+[\s]+)+\b(ALLEY|ALY|AVENUE|AVE|BEND|BND|BLUFFS?|BLFS?|BOULEVARD|BLVD|BRANCH|BR|CENTERS?|CTRS?|CIRCLES?|CIRS?|CLIFFS?|CLFS?|COURTS?|CTS?|COVES?|CVS?|CREEK|CRK|CRESCENT|CRES|CREST|CRST|CROSSING|XING|DRIVES?|DRS?|EXPRESSWAY|EXPY|FREEWAY|FWY|HEIGHTS|HTS|HIGHWAY|HWY|HILLS?|HLS?|LANE|LN|LOOP|MANORS?|MNRS?|MOTORWAY|MTWY|MOUNT|MT|PARKS?|PARKWAYS?|PKWY|PASS|PLACE|PL|PLAZA|PLZ|POINTS?|PTS?|RIDGES?|RDGS?|ROADS?|RDS?|ROUTE|RTE?|SHOALS?|SHLS?|SHORES?|SHRS?|SPRINGS?|SPGS?|SPURS?|STREETS?|STS?|SUMMIT|SMT|TERRACE|TER|THROUGHWAY|TRWY|TRAFFICWAY|TRFY|TRAIL|TRL|TURNPIKE|TPKE|VALLEYS?|VLYS?|WAYS?)+(?:[\.\-\s\,]?)*((APARTMENT|APT|APPT|#|NUMBER|NUM|FLOOR|FL|\s)?(\d)*)\b/ig;

	  return regex.test(str);
	};
	is.street = is.address = is.streetAddress;

	/**
	 * Test if a string resembles a US Zip code,
	 * no regular expression will be perfect for this,
	 * as there are many numbers that aren't valid zip codes
	 * @param {String || Number} the string or number literal to test
	 * @return true if zipcode like, false otherwise
	 */
	is.zipCode = function(str) {
	  if (is.undefined(str) || !(is.string(str) || is.number(str)))
	    return false;

	  var zip = /^\d{5}(?:-\d{4})?$/;
	  return zip.test(str);
	};
	is.zip = is.zipCode;

	/**
	 * Test if a string contains a US phone number
	 * @param {String} the string to search
	 * @return true if str contains a phone number, false otherwise.
	 */
	 is.phoneNumber = function(str){
	   if (!is.string(str))
	    return false;
	   var nums = /(?:(?:\+?1\s*(?:[.-]\s*)?)?(?:(\(?)(?:(\s*([2-9]1[02-9]|[2-9][02-8]1|[2-9][02-8][02-9]‌​)\s*)|([2-9]1[02-9]|[2-9][02-8]1|[2-9][02-8][02-9]))\)?)\s*(?:[.-]\s*)?)?([2-9]1[02-‌​9]|[2-9][02-9]1|[2-9][02-9]{2})\s*(?:[.-]\s*)?([0-9]{4})/g;
	   return nums.test(str);
	 };
	 is.phone = is.phoneNumber;

	/**
	 * Test is a string is a valid URL
	 * @param {string} val - the possible url to check
	 * @return true if str contains a phone number, false otherwise.
	 */
	var isUrl = requireIsUrl();
	is.url = function(val) {
	    return isUrl(val);
	};
	is.uri = is.url;

	is.enumerator = function(val, ary){
	  var value = false;

	  if (!is.defined(val) || !is.defined(ary) || !is.arrayLike(ary))
	    return value;

	  for (var i = 0, len = ary.length; i < len; i++) {
	    if (is.equal(val, ary[i])) {
	      value = true;
	      break;
	    }
	  }
	  return value;
	};
	is.enum = is.inArray = is.enumerator; 
} (is2));

var src = {exports: {}};

var browser = {exports: {}};

/**
 * Helpers.
 */

var ms;
var hasRequiredMs;

function requireMs () {
	if (hasRequiredMs) return ms;
	hasRequiredMs = 1;
	var s = 1000;
	var m = s * 60;
	var h = m * 60;
	var d = h * 24;
	var w = d * 7;
	var y = d * 365.25;

	/**
	 * Parse or format the given `val`.
	 *
	 * Options:
	 *
	 *  - `long` verbose formatting [false]
	 *
	 * @param {String|Number} val
	 * @param {Object} [options]
	 * @throws {Error} throw an error if val is not a non-empty string or a number
	 * @return {String|Number}
	 * @api public
	 */

	ms = function(val, options) {
	  options = options || {};
	  var type = typeof val;
	  if (type === 'string' && val.length > 0) {
	    return parse(val);
	  } else if (type === 'number' && isFinite(val)) {
	    return options.long ? fmtLong(val) : fmtShort(val);
	  }
	  throw new Error(
	    'val is not a non-empty string or a valid number. val=' +
	      JSON.stringify(val)
	  );
	};

	/**
	 * Parse the given `str` and return milliseconds.
	 *
	 * @param {String} str
	 * @return {Number}
	 * @api private
	 */

	function parse(str) {
	  str = String(str);
	  if (str.length > 100) {
	    return;
	  }
	  var match = /^(-?(?:\d+)?\.?\d+) *(milliseconds?|msecs?|ms|seconds?|secs?|s|minutes?|mins?|m|hours?|hrs?|h|days?|d|weeks?|w|years?|yrs?|y)?$/i.exec(
	    str
	  );
	  if (!match) {
	    return;
	  }
	  var n = parseFloat(match[1]);
	  var type = (match[2] || 'ms').toLowerCase();
	  switch (type) {
	    case 'years':
	    case 'year':
	    case 'yrs':
	    case 'yr':
	    case 'y':
	      return n * y;
	    case 'weeks':
	    case 'week':
	    case 'w':
	      return n * w;
	    case 'days':
	    case 'day':
	    case 'd':
	      return n * d;
	    case 'hours':
	    case 'hour':
	    case 'hrs':
	    case 'hr':
	    case 'h':
	      return n * h;
	    case 'minutes':
	    case 'minute':
	    case 'mins':
	    case 'min':
	    case 'm':
	      return n * m;
	    case 'seconds':
	    case 'second':
	    case 'secs':
	    case 'sec':
	    case 's':
	      return n * s;
	    case 'milliseconds':
	    case 'millisecond':
	    case 'msecs':
	    case 'msec':
	    case 'ms':
	      return n;
	    default:
	      return undefined;
	  }
	}

	/**
	 * Short format for `ms`.
	 *
	 * @param {Number} ms
	 * @return {String}
	 * @api private
	 */

	function fmtShort(ms) {
	  var msAbs = Math.abs(ms);
	  if (msAbs >= d) {
	    return Math.round(ms / d) + 'd';
	  }
	  if (msAbs >= h) {
	    return Math.round(ms / h) + 'h';
	  }
	  if (msAbs >= m) {
	    return Math.round(ms / m) + 'm';
	  }
	  if (msAbs >= s) {
	    return Math.round(ms / s) + 's';
	  }
	  return ms + 'ms';
	}

	/**
	 * Long format for `ms`.
	 *
	 * @param {Number} ms
	 * @return {String}
	 * @api private
	 */

	function fmtLong(ms) {
	  var msAbs = Math.abs(ms);
	  if (msAbs >= d) {
	    return plural(ms, msAbs, d, 'day');
	  }
	  if (msAbs >= h) {
	    return plural(ms, msAbs, h, 'hour');
	  }
	  if (msAbs >= m) {
	    return plural(ms, msAbs, m, 'minute');
	  }
	  if (msAbs >= s) {
	    return plural(ms, msAbs, s, 'second');
	  }
	  return ms + ' ms';
	}

	/**
	 * Pluralization helper.
	 */

	function plural(ms, msAbs, n, name) {
	  var isPlural = msAbs >= n * 1.5;
	  return Math.round(ms / n) + ' ' + name + (isPlural ? 's' : '');
	}
	return ms;
}

var common;
var hasRequiredCommon;

function requireCommon () {
	if (hasRequiredCommon) return common;
	hasRequiredCommon = 1;
	/**
	 * This is the common logic for both the Node.js and web browser
	 * implementations of `debug()`.
	 */

	function setup(env) {
		createDebug.debug = createDebug;
		createDebug.default = createDebug;
		createDebug.coerce = coerce;
		createDebug.disable = disable;
		createDebug.enable = enable;
		createDebug.enabled = enabled;
		createDebug.humanize = requireMs();
		createDebug.destroy = destroy;

		Object.keys(env).forEach(key => {
			createDebug[key] = env[key];
		});

		/**
		* The currently active debug mode names, and names to skip.
		*/

		createDebug.names = [];
		createDebug.skips = [];

		/**
		* Map of special "%n" handling functions, for the debug "format" argument.
		*
		* Valid key names are a single, lower or upper-case letter, i.e. "n" and "N".
		*/
		createDebug.formatters = {};

		/**
		* Selects a color for a debug namespace
		* @param {String} namespace The namespace string for the for the debug instance to be colored
		* @return {Number|String} An ANSI color code for the given namespace
		* @api private
		*/
		function selectColor(namespace) {
			let hash = 0;

			for (let i = 0; i < namespace.length; i++) {
				hash = ((hash << 5) - hash) + namespace.charCodeAt(i);
				hash |= 0; // Convert to 32bit integer
			}

			return createDebug.colors[Math.abs(hash) % createDebug.colors.length];
		}
		createDebug.selectColor = selectColor;

		/**
		* Create a debugger with the given `namespace`.
		*
		* @param {String} namespace
		* @return {Function}
		* @api public
		*/
		function createDebug(namespace) {
			let prevTime;
			let enableOverride = null;

			function debug(...args) {
				// Disabled?
				if (!debug.enabled) {
					return;
				}

				const self = debug;

				// Set `diff` timestamp
				const curr = Number(new Date());
				const ms = curr - (prevTime || curr);
				self.diff = ms;
				self.prev = prevTime;
				self.curr = curr;
				prevTime = curr;

				args[0] = createDebug.coerce(args[0]);

				if (typeof args[0] !== 'string') {
					// Anything else let's inspect with %O
					args.unshift('%O');
				}

				// Apply any `formatters` transformations
				let index = 0;
				args[0] = args[0].replace(/%([a-zA-Z%])/g, (match, format) => {
					// If we encounter an escaped % then don't increase the array index
					if (match === '%%') {
						return '%';
					}
					index++;
					const formatter = createDebug.formatters[format];
					if (typeof formatter === 'function') {
						const val = args[index];
						match = formatter.call(self, val);

						// Now we need to remove `args[index]` since it's inlined in the `format`
						args.splice(index, 1);
						index--;
					}
					return match;
				});

				// Apply env-specific formatting (colors, etc.)
				createDebug.formatArgs.call(self, args);

				const logFn = self.log || createDebug.log;
				logFn.apply(self, args);
			}

			debug.namespace = namespace;
			debug.useColors = createDebug.useColors();
			debug.color = createDebug.selectColor(namespace);
			debug.extend = extend;
			debug.destroy = createDebug.destroy; // XXX Temporary. Will be removed in the next major release.

			Object.defineProperty(debug, 'enabled', {
				enumerable: true,
				configurable: false,
				get: () => enableOverride === null ? createDebug.enabled(namespace) : enableOverride,
				set: v => {
					enableOverride = v;
				}
			});

			// Env-specific initialization logic for debug instances
			if (typeof createDebug.init === 'function') {
				createDebug.init(debug);
			}

			return debug;
		}

		function extend(namespace, delimiter) {
			const newDebug = createDebug(this.namespace + (typeof delimiter === 'undefined' ? ':' : delimiter) + namespace);
			newDebug.log = this.log;
			return newDebug;
		}

		/**
		* Enables a debug mode by namespaces. This can include modes
		* separated by a colon and wildcards.
		*
		* @param {String} namespaces
		* @api public
		*/
		function enable(namespaces) {
			createDebug.save(namespaces);

			createDebug.names = [];
			createDebug.skips = [];

			let i;
			const split = (typeof namespaces === 'string' ? namespaces : '').split(/[\s,]+/);
			const len = split.length;

			for (i = 0; i < len; i++) {
				if (!split[i]) {
					// ignore empty strings
					continue;
				}

				namespaces = split[i].replace(/\*/g, '.*?');

				if (namespaces[0] === '-') {
					createDebug.skips.push(new RegExp('^' + namespaces.substr(1) + '$'));
				} else {
					createDebug.names.push(new RegExp('^' + namespaces + '$'));
				}
			}
		}

		/**
		* Disable debug output.
		*
		* @return {String} namespaces
		* @api public
		*/
		function disable() {
			const namespaces = [
				...createDebug.names.map(toNamespace),
				...createDebug.skips.map(toNamespace).map(namespace => '-' + namespace)
			].join(',');
			createDebug.enable('');
			return namespaces;
		}

		/**
		* Returns true if the given mode name is enabled, false otherwise.
		*
		* @param {String} name
		* @return {Boolean}
		* @api public
		*/
		function enabled(name) {
			if (name[name.length - 1] === '*') {
				return true;
			}

			let i;
			let len;

			for (i = 0, len = createDebug.skips.length; i < len; i++) {
				if (createDebug.skips[i].test(name)) {
					return false;
				}
			}

			for (i = 0, len = createDebug.names.length; i < len; i++) {
				if (createDebug.names[i].test(name)) {
					return true;
				}
			}

			return false;
		}

		/**
		* Convert regexp to namespace
		*
		* @param {RegExp} regxep
		* @return {String} namespace
		* @api private
		*/
		function toNamespace(regexp) {
			return regexp.toString()
				.substring(2, regexp.toString().length - 2)
				.replace(/\.\*\?$/, '*');
		}

		/**
		* Coerce `val`.
		*
		* @param {Mixed} val
		* @return {Mixed}
		* @api private
		*/
		function coerce(val) {
			if (val instanceof Error) {
				return val.stack || val.message;
			}
			return val;
		}

		/**
		* XXX DO NOT USE. This is a temporary stub function.
		* XXX It WILL be removed in the next major release.
		*/
		function destroy() {
			console.warn('Instance method `debug.destroy()` is deprecated and no longer does anything. It will be removed in the next major version of `debug`.');
		}

		createDebug.enable(createDebug.load());

		return createDebug;
	}

	common = setup;
	return common;
}

/* eslint-env browser */
browser.exports;

var hasRequiredBrowser;

function requireBrowser () {
	if (hasRequiredBrowser) return browser.exports;
	hasRequiredBrowser = 1;
	(function (module, exports) {
		/**
		 * This is the web browser implementation of `debug()`.
		 */

		exports.formatArgs = formatArgs;
		exports.save = save;
		exports.load = load;
		exports.useColors = useColors;
		exports.storage = localstorage();
		exports.destroy = (() => {
			let warned = false;

			return () => {
				if (!warned) {
					warned = true;
					console.warn('Instance method `debug.destroy()` is deprecated and no longer does anything. It will be removed in the next major version of `debug`.');
				}
			};
		})();

		/**
		 * Colors.
		 */

		exports.colors = [
			'#0000CC',
			'#0000FF',
			'#0033CC',
			'#0033FF',
			'#0066CC',
			'#0066FF',
			'#0099CC',
			'#0099FF',
			'#00CC00',
			'#00CC33',
			'#00CC66',
			'#00CC99',
			'#00CCCC',
			'#00CCFF',
			'#3300CC',
			'#3300FF',
			'#3333CC',
			'#3333FF',
			'#3366CC',
			'#3366FF',
			'#3399CC',
			'#3399FF',
			'#33CC00',
			'#33CC33',
			'#33CC66',
			'#33CC99',
			'#33CCCC',
			'#33CCFF',
			'#6600CC',
			'#6600FF',
			'#6633CC',
			'#6633FF',
			'#66CC00',
			'#66CC33',
			'#9900CC',
			'#9900FF',
			'#9933CC',
			'#9933FF',
			'#99CC00',
			'#99CC33',
			'#CC0000',
			'#CC0033',
			'#CC0066',
			'#CC0099',
			'#CC00CC',
			'#CC00FF',
			'#CC3300',
			'#CC3333',
			'#CC3366',
			'#CC3399',
			'#CC33CC',
			'#CC33FF',
			'#CC6600',
			'#CC6633',
			'#CC9900',
			'#CC9933',
			'#CCCC00',
			'#CCCC33',
			'#FF0000',
			'#FF0033',
			'#FF0066',
			'#FF0099',
			'#FF00CC',
			'#FF00FF',
			'#FF3300',
			'#FF3333',
			'#FF3366',
			'#FF3399',
			'#FF33CC',
			'#FF33FF',
			'#FF6600',
			'#FF6633',
			'#FF9900',
			'#FF9933',
			'#FFCC00',
			'#FFCC33'
		];

		/**
		 * Currently only WebKit-based Web Inspectors, Firefox >= v31,
		 * and the Firebug extension (any Firefox version) are known
		 * to support "%c" CSS customizations.
		 *
		 * TODO: add a `localStorage` variable to explicitly enable/disable colors
		 */

		// eslint-disable-next-line complexity
		function useColors() {
			// NB: In an Electron preload script, document will be defined but not fully
			// initialized. Since we know we're in Chrome, we'll just detect this case
			// explicitly
			if (typeof window !== 'undefined' && window.process && (window.process.type === 'renderer' || window.process.__nwjs)) {
				return true;
			}

			// Internet Explorer and Edge do not support colors.
			if (typeof navigator !== 'undefined' && navigator.userAgent && navigator.userAgent.toLowerCase().match(/(edge|trident)\/(\d+)/)) {
				return false;
			}

			// Is webkit? http://stackoverflow.com/a/16459606/376773
			// document is undefined in react-native: https://github.com/facebook/react-native/pull/1632
			return (typeof document !== 'undefined' && document.documentElement && document.documentElement.style && document.documentElement.style.WebkitAppearance) ||
				// Is firebug? http://stackoverflow.com/a/398120/376773
				(typeof window !== 'undefined' && window.console && (window.console.firebug || (window.console.exception && window.console.table))) ||
				// Is firefox >= v31?
				// https://developer.mozilla.org/en-US/docs/Tools/Web_Console#Styling_messages
				(typeof navigator !== 'undefined' && navigator.userAgent && navigator.userAgent.toLowerCase().match(/firefox\/(\d+)/) && parseInt(RegExp.$1, 10) >= 31) ||
				// Double check webkit in userAgent just in case we are in a worker
				(typeof navigator !== 'undefined' && navigator.userAgent && navigator.userAgent.toLowerCase().match(/applewebkit\/(\d+)/));
		}

		/**
		 * Colorize log arguments if enabled.
		 *
		 * @api public
		 */

		function formatArgs(args) {
			args[0] = (this.useColors ? '%c' : '') +
				this.namespace +
				(this.useColors ? ' %c' : ' ') +
				args[0] +
				(this.useColors ? '%c ' : ' ') +
				'+' + module.exports.humanize(this.diff);

			if (!this.useColors) {
				return;
			}

			const c = 'color: ' + this.color;
			args.splice(1, 0, c, 'color: inherit');

			// The final "%c" is somewhat tricky, because there could be other
			// arguments passed either before or after the %c, so we need to
			// figure out the correct index to insert the CSS into
			let index = 0;
			let lastC = 0;
			args[0].replace(/%[a-zA-Z%]/g, match => {
				if (match === '%%') {
					return;
				}
				index++;
				if (match === '%c') {
					// We only are interested in the *last* %c
					// (the user may have provided their own)
					lastC = index;
				}
			});

			args.splice(lastC, 0, c);
		}

		/**
		 * Invokes `console.debug()` when available.
		 * No-op when `console.debug` is not a "function".
		 * If `console.debug` is not available, falls back
		 * to `console.log`.
		 *
		 * @api public
		 */
		exports.log = console.debug || console.log || (() => {});

		/**
		 * Save `namespaces`.
		 *
		 * @param {String} namespaces
		 * @api private
		 */
		function save(namespaces) {
			try {
				if (namespaces) {
					exports.storage.setItem('debug', namespaces);
				} else {
					exports.storage.removeItem('debug');
				}
			} catch (error) {
				// Swallow
				// XXX (@Qix-) should we be logging these?
			}
		}

		/**
		 * Load `namespaces`.
		 *
		 * @return {String} returns the previously persisted debug modes
		 * @api private
		 */
		function load() {
			let r;
			try {
				r = exports.storage.getItem('debug');
			} catch (error) {
				// Swallow
				// XXX (@Qix-) should we be logging these?
			}

			// If debug isn't set in LS, and we're in Electron, try to load $DEBUG
			if (!r && typeof process !== 'undefined' && 'env' in process) {
				r = process.env.DEBUG;
			}

			return r;
		}

		/**
		 * Localstorage attempts to return the localstorage.
		 *
		 * This is necessary because safari throws
		 * when a user disables cookies/localstorage
		 * and you attempt to access it.
		 *
		 * @return {LocalStorage}
		 * @api private
		 */

		function localstorage() {
			try {
				// TVMLKit (Apple TV JS Runtime) does not have a window object, just localStorage in the global context
				// The Browser also has localStorage in the global context.
				return localStorage;
			} catch (error) {
				// Swallow
				// XXX (@Qix-) should we be logging these?
			}
		}

		module.exports = requireCommon()(exports);

		const {formatters} = module.exports;

		/**
		 * Map %j to `JSON.stringify()`, since no Web Inspectors do that by default.
		 */

		formatters.j = function (v) {
			try {
				return JSON.stringify(v);
			} catch (error) {
				return '[UnexpectedJSONParseError]: ' + error.message;
			}
		}; 
	} (browser, browser.exports));
	return browser.exports;
}

var node = {exports: {}};

var hasFlag;
var hasRequiredHasFlag;

function requireHasFlag () {
	if (hasRequiredHasFlag) return hasFlag;
	hasRequiredHasFlag = 1;

	hasFlag = (flag, argv = process.argv) => {
		const prefix = flag.startsWith('-') ? '' : (flag.length === 1 ? '-' : '--');
		const position = argv.indexOf(prefix + flag);
		const terminatorPosition = argv.indexOf('--');
		return position !== -1 && (terminatorPosition === -1 || position < terminatorPosition);
	};
	return hasFlag;
}

var supportsColor_1;
var hasRequiredSupportsColor;

function requireSupportsColor () {
	if (hasRequiredSupportsColor) return supportsColor_1;
	hasRequiredSupportsColor = 1;
	const os = require$$0__default["default"];
	const tty = require$$1__default["default"];
	const hasFlag = requireHasFlag();

	const {env} = process;

	let forceColor;
	if (hasFlag('no-color') ||
		hasFlag('no-colors') ||
		hasFlag('color=false') ||
		hasFlag('color=never')) {
		forceColor = 0;
	} else if (hasFlag('color') ||
		hasFlag('colors') ||
		hasFlag('color=true') ||
		hasFlag('color=always')) {
		forceColor = 1;
	}

	if ('FORCE_COLOR' in env) {
		if (env.FORCE_COLOR === 'true') {
			forceColor = 1;
		} else if (env.FORCE_COLOR === 'false') {
			forceColor = 0;
		} else {
			forceColor = env.FORCE_COLOR.length === 0 ? 1 : Math.min(parseInt(env.FORCE_COLOR, 10), 3);
		}
	}

	function translateLevel(level) {
		if (level === 0) {
			return false;
		}

		return {
			level,
			hasBasic: true,
			has256: level >= 2,
			has16m: level >= 3
		};
	}

	function supportsColor(haveStream, streamIsTTY) {
		if (forceColor === 0) {
			return 0;
		}

		if (hasFlag('color=16m') ||
			hasFlag('color=full') ||
			hasFlag('color=truecolor')) {
			return 3;
		}

		if (hasFlag('color=256')) {
			return 2;
		}

		if (haveStream && !streamIsTTY && forceColor === undefined) {
			return 0;
		}

		const min = forceColor || 0;

		if (env.TERM === 'dumb') {
			return min;
		}

		if (process.platform === 'win32') {
			// Windows 10 build 10586 is the first Windows release that supports 256 colors.
			// Windows 10 build 14931 is the first release that supports 16m/TrueColor.
			const osRelease = os.release().split('.');
			if (
				Number(osRelease[0]) >= 10 &&
				Number(osRelease[2]) >= 10586
			) {
				return Number(osRelease[2]) >= 14931 ? 3 : 2;
			}

			return 1;
		}

		if ('CI' in env) {
			if (['TRAVIS', 'CIRCLECI', 'APPVEYOR', 'GITLAB_CI', 'GITHUB_ACTIONS', 'BUILDKITE'].some(sign => sign in env) || env.CI_NAME === 'codeship') {
				return 1;
			}

			return min;
		}

		if ('TEAMCITY_VERSION' in env) {
			return /^(9\.(0*[1-9]\d*)\.|\d{2,}\.)/.test(env.TEAMCITY_VERSION) ? 1 : 0;
		}

		if (env.COLORTERM === 'truecolor') {
			return 3;
		}

		if ('TERM_PROGRAM' in env) {
			const version = parseInt((env.TERM_PROGRAM_VERSION || '').split('.')[0], 10);

			switch (env.TERM_PROGRAM) {
				case 'iTerm.app':
					return version >= 3 ? 3 : 2;
				case 'Apple_Terminal':
					return 2;
				// No default
			}
		}

		if (/-256(color)?$/i.test(env.TERM)) {
			return 2;
		}

		if (/^screen|^xterm|^vt100|^vt220|^rxvt|color|ansi|cygwin|linux/i.test(env.TERM)) {
			return 1;
		}

		if ('COLORTERM' in env) {
			return 1;
		}

		return min;
	}

	function getSupportLevel(stream) {
		const level = supportsColor(stream, stream && stream.isTTY);
		return translateLevel(level);
	}

	supportsColor_1 = {
		supportsColor: getSupportLevel,
		stdout: translateLevel(supportsColor(true, tty.isatty(1))),
		stderr: translateLevel(supportsColor(true, tty.isatty(2)))
	};
	return supportsColor_1;
}

/**
 * Module dependencies.
 */
node.exports;

var hasRequiredNode;

function requireNode () {
	if (hasRequiredNode) return node.exports;
	hasRequiredNode = 1;
	(function (module, exports) {
		const tty = require$$1__default["default"];
		const util = require$$1__default$1["default"];

		/**
		 * This is the Node.js implementation of `debug()`.
		 */

		exports.init = init;
		exports.log = log;
		exports.formatArgs = formatArgs;
		exports.save = save;
		exports.load = load;
		exports.useColors = useColors;
		exports.destroy = util.deprecate(
			() => {},
			'Instance method `debug.destroy()` is deprecated and no longer does anything. It will be removed in the next major version of `debug`.'
		);

		/**
		 * Colors.
		 */

		exports.colors = [6, 2, 3, 4, 5, 1];

		try {
			// Optional dependency (as in, doesn't need to be installed, NOT like optionalDependencies in package.json)
			// eslint-disable-next-line import/no-extraneous-dependencies
			const supportsColor = requireSupportsColor();

			if (supportsColor && (supportsColor.stderr || supportsColor).level >= 2) {
				exports.colors = [
					20,
					21,
					26,
					27,
					32,
					33,
					38,
					39,
					40,
					41,
					42,
					43,
					44,
					45,
					56,
					57,
					62,
					63,
					68,
					69,
					74,
					75,
					76,
					77,
					78,
					79,
					80,
					81,
					92,
					93,
					98,
					99,
					112,
					113,
					128,
					129,
					134,
					135,
					148,
					149,
					160,
					161,
					162,
					163,
					164,
					165,
					166,
					167,
					168,
					169,
					170,
					171,
					172,
					173,
					178,
					179,
					184,
					185,
					196,
					197,
					198,
					199,
					200,
					201,
					202,
					203,
					204,
					205,
					206,
					207,
					208,
					209,
					214,
					215,
					220,
					221
				];
			}
		} catch (error) {
			// Swallow - we only care if `supports-color` is available; it doesn't have to be.
		}

		/**
		 * Build up the default `inspectOpts` object from the environment variables.
		 *
		 *   $ DEBUG_COLORS=no DEBUG_DEPTH=10 DEBUG_SHOW_HIDDEN=enabled node script.js
		 */

		exports.inspectOpts = Object.keys(process.env).filter(key => {
			return /^debug_/i.test(key);
		}).reduce((obj, key) => {
			// Camel-case
			const prop = key
				.substring(6)
				.toLowerCase()
				.replace(/_([a-z])/g, (_, k) => {
					return k.toUpperCase();
				});

			// Coerce string value into JS value
			let val = process.env[key];
			if (/^(yes|on|true|enabled)$/i.test(val)) {
				val = true;
			} else if (/^(no|off|false|disabled)$/i.test(val)) {
				val = false;
			} else if (val === 'null') {
				val = null;
			} else {
				val = Number(val);
			}

			obj[prop] = val;
			return obj;
		}, {});

		/**
		 * Is stdout a TTY? Colored output is enabled when `true`.
		 */

		function useColors() {
			return 'colors' in exports.inspectOpts ?
				Boolean(exports.inspectOpts.colors) :
				tty.isatty(process.stderr.fd);
		}

		/**
		 * Adds ANSI color escape codes if enabled.
		 *
		 * @api public
		 */

		function formatArgs(args) {
			const {namespace: name, useColors} = this;

			if (useColors) {
				const c = this.color;
				const colorCode = '\u001B[3' + (c < 8 ? c : '8;5;' + c);
				const prefix = `  ${colorCode};1m${name} \u001B[0m`;

				args[0] = prefix + args[0].split('\n').join('\n' + prefix);
				args.push(colorCode + 'm+' + module.exports.humanize(this.diff) + '\u001B[0m');
			} else {
				args[0] = getDate() + name + ' ' + args[0];
			}
		}

		function getDate() {
			if (exports.inspectOpts.hideDate) {
				return '';
			}
			return new Date().toISOString() + ' ';
		}

		/**
		 * Invokes `util.format()` with the specified arguments and writes to stderr.
		 */

		function log(...args) {
			return process.stderr.write(util.format(...args) + '\n');
		}

		/**
		 * Save `namespaces`.
		 *
		 * @param {String} namespaces
		 * @api private
		 */
		function save(namespaces) {
			if (namespaces) {
				process.env.DEBUG = namespaces;
			} else {
				// If you set a process.env field to null or undefined, it gets cast to the
				// string 'null' or 'undefined'. Just delete instead.
				delete process.env.DEBUG;
			}
		}

		/**
		 * Load `namespaces`.
		 *
		 * @return {String} returns the previously persisted debug modes
		 * @api private
		 */

		function load() {
			return process.env.DEBUG;
		}

		/**
		 * Init logic for `debug` instances.
		 *
		 * Create a new `inspectOpts` object in case `useColors` is set
		 * differently for a particular `debug` instance.
		 */

		function init(debug) {
			debug.inspectOpts = {};

			const keys = Object.keys(exports.inspectOpts);
			for (let i = 0; i < keys.length; i++) {
				debug.inspectOpts[keys[i]] = exports.inspectOpts[keys[i]];
			}
		}

		module.exports = requireCommon()(exports);

		const {formatters} = module.exports;

		/**
		 * Map %o to `util.inspect()`, all on a single line.
		 */

		formatters.o = function (v) {
			this.inspectOpts.colors = this.useColors;
			return util.inspect(v, this.inspectOpts)
				.split('\n')
				.map(str => str.trim())
				.join(' ');
		};

		/**
		 * Map %O to `util.inspect()`, allowing multiple lines if needed.
		 */

		formatters.O = function (v) {
			this.inspectOpts.colors = this.useColors;
			return util.inspect(v, this.inspectOpts);
		}; 
	} (node, node.exports));
	return node.exports;
}

/**
 * Detect Electron renderer / nwjs process, which is node, but we should
 * treat as a browser.
 */

if (typeof process === 'undefined' || process.type === 'renderer' || process.browser === true || process.__nwjs) {
	src.exports = requireBrowser();
} else {
	src.exports = requireNode();
}

var srcExports = src.exports;

/**
 * @fileOverview
 * A simple promises-based check to see if a TCP port is already in use.
 */

// define the exports first to avoid cyclic dependencies.
tcpPortUsed.check = check;
tcpPortUsed.waitUntilFreeOnHost = waitUntilFreeOnHost;
tcpPortUsed.waitUntilFree = waitUntilFree;
tcpPortUsed.waitUntilUsedOnHost = waitUntilUsedOnHost;
tcpPortUsed.waitUntilUsed = waitUntilUsed;
tcpPortUsed.waitForStatus = waitForStatus;

var is = is2;
var net = require$$1__default$2["default"];
var util = require$$1__default$1["default"];
var debug = srcExports('tcp-port-used');

// Global Values
var TIMEOUT = 2000;
var RETRYTIME = 250;

function getDeferred() {
    var resolve, reject, promise = new Promise(function(res, rej) {
        resolve = res;
        reject = rej;
    });

    return {
        resolve: resolve,
        reject: reject,
        promise: promise
    };
}

/**
 * Creates an options object from all the possible arguments
 * @private
 * @param {Number} port a valid TCP port number
 * @param {String} host The DNS name or IP address.
 * @param {Boolean} status The desired in use status to wait for: false === not in use, true === in use
 * @param {Number} retryTimeMs the retry interval in milliseconds - defaultis is 200ms
 * @param {Number} timeOutMs the amount of time to wait until port is free default is 1000ms
 * @return {Object} An options object with all the above parameters as properties.
 */
function makeOptionsObj(port, host, inUse, retryTimeMs, timeOutMs) {
    var opts = {};
    opts.port = port;
    opts.host = host;
    opts.inUse = inUse;
    opts.retryTimeMs = retryTimeMs;
    opts.timeOutMs = timeOutMs;
    return opts;
}

/**
 * Checks if a TCP port is in use by creating the socket and binding it to the
 * target port. Once bound, successfully, it's assume the port is availble.
 * After the socket is closed or in error, the promise is resolved.
 * Note: you have to be super user to correctly test system ports (0-1023).
 * @param {Number|Object} port The port you are curious to see if available. If an object, must have the parameters as properties.
 * @param {String} [host] May be a DNS name or IP address. Default '127.0.0.1'
 * @return {Object} A deferred Q promise.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.check(22, '127.0.0.1')
 * .then(function(inUse) {
 *    debug('Port 22 usage: '+inUse);
 * }, function(err) {
 *    console.error('Error on check: '+util.inspect(err));
 * });
 */
function check(port, host) {

    var deferred = getDeferred();
    var inUse = true;
    var client;

    var opts;
    if (!is.obj(port)) {
        opts = makeOptionsObj(port, host);
    } else {
        opts = port;
    }

    if (!is.port(opts.port)) {
        debug('Error invalid port: '+util.inspect(opts.port));
        deferred.reject(new Error('invalid port: '+util.inspect(opts.port)));
        return deferred.promise;
    }

    if (is.nullOrUndefined(opts.host)) {
        debug('set host address to default 127.0.0.1');
        opts.host = '127.0.0.1';
    }

    function cleanUp() {
        if (client) {
            client.removeAllListeners('connect');
            client.removeAllListeners('error');
            client.end();
            client.destroy();
            client.unref();
        }
        //debug('listeners removed from client socket');
    }

    function onConnectCb() {
        //debug('check - promise resolved - in use');
        deferred.resolve(inUse);
        cleanUp();
    }

    function onErrorCb(err) {
        if (err.code !== 'ECONNREFUSED') {
            //debug('check - promise rejected, error: '+err.message);
            deferred.reject(err);
        } else {
            //debug('ECONNREFUSED');
            inUse = false;
            //debug('check - promise resolved - not in use');
            deferred.resolve(inUse);
        }
        cleanUp();
    }

    client = new net.Socket();
    client.once('connect', onConnectCb);
    client.once('error', onErrorCb);
    client.connect({port: opts.port, host: opts.host}, function() {});

    return deferred.promise;
}

/**
 * Creates a deferred promise and fulfills it only when the socket's usage
 * equals status in terms of 'in use' (false === not in use, true === in use).
 * Will retry on an interval specified in retryTimeMs.  Note: you have to be
 * super user to correctly test system ports (0-1023).
 * @param {Number|Object} port a valid TCP port number, if an object, has all the parameters described as properties.
 * @param {String} host The DNS name or IP address.
 * @param {Boolean} status The desired in use status to wait for false === not in use, true === in use
 * @param {Number} [retryTimeMs] the retry interval in milliseconds - defaultis is 200ms
 * @param {Number} [timeOutMs] the amount of time to wait until port is free default is 1000ms
 * @return {Object} A deferred promise from the Q library.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.waitForStatus(44204, 'some.host.com', true, 500, 4000)
 * .then(function() {
 *     console.log('Port 44204 is now in use.');
 * }, function(err) {
 *     console.log('Error: ', error.message);
 * });
 */
function waitForStatus(port, host, inUse, retryTimeMs, timeOutMs) {

    var deferred = getDeferred();
    var timeoutId;
    var timedout = false;
    var retryId;

    // the first arument may be an object, if it is not, make an object
    var opts;
    if (is.obj(port)) {
        opts = port;
    } else {
        opts = makeOptionsObj(port, host, inUse, retryTimeMs, timeOutMs);
    }

    //debug('opts:'+util.inspect(opts);

    if (!is.bool(opts.inUse)) {
        deferred.reject(new Error('inUse must be a boolean'));
        return deferred.promise;
    }

    if (!is.positiveInt(opts.retryTimeMs)) {
        opts.retryTimeMs = RETRYTIME;
        debug('set retryTime to default '+RETRYTIME+'ms');
    }

    if (!is.positiveInt(opts.timeOutMs)) {
        opts.timeOutMs = TIMEOUT;
        debug('set timeOutMs to default '+TIMEOUT+'ms');
    }

    function cleanUp() {
        if (timeoutId) {
            clearTimeout(timeoutId);
        }
        if (retryId) {
            clearTimeout(retryId);
        }
    }

    function timeoutFunc() {
        timedout = true;
        cleanUp();
        deferred.reject(new Error('timeout'));
    }
    timeoutId = setTimeout(timeoutFunc, opts.timeOutMs);

    function doCheck() {
        check(opts.port, opts.host)
        .then(function(inUse) {
            if (timedout) {
                return;
            }
            //debug('doCheck inUse: '+inUse);
            //debug('doCheck opts.inUse: '+opts.inUse);
            if (inUse === opts.inUse) {
                deferred.resolve();
                cleanUp();
                return;
            } else {
                retryId = setTimeout(function() { doCheck(); }, opts.retryTimeMs);
                return;
            }
        }, function(err) {
            if (timedout) {
                return;
            }
            deferred.reject(err);
            cleanUp();
        });
    }

    doCheck();
    return deferred.promise;
}

/**
 * Creates a deferred promise and fulfills it only when the socket is free.
 * Will retry on an interval specified in retryTimeMs.
 * Note: you have to be super user to correctly test system ports (0-1023).
 * @param {Number} port a valid TCP port number
 * @param {String} [host] The hostname or IP address of where the socket is.
 * @param {Number} [retryTimeMs] the retry interval in milliseconds - defaultis is 100ms.
 * @param {Number} [timeOutMs] the amount of time to wait until port is free. Default 300ms.
 * @return {Object} A deferred promise from the q library.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.waitUntilFreeOnHost(44203, 'some.host.com', 500, 4000)
 * .then(function() {
 *     console.log('Port 44203 is now free.');
 *  }, function(err) {
 *     console.loh('Error: ', error.message);
 *  });
 */
function waitUntilFreeOnHost(port, host, retryTimeMs, timeOutMs) {

    // the first arument may be an object, if it is not, make an object
    var opts;
    if (is.obj(port)) {
        opts = port;
        opts.inUse = false;
    } else {
        opts = makeOptionsObj(port, host, false, retryTimeMs, timeOutMs);
    }

    return waitForStatus(opts);
}

/**
 * For compatibility with previous version of the module, that did not provide
 * arguements for hostnames. The host is set to the localhost '127.0.0.1'.
 * @param {Number|Object} port a valid TCP port number. If an object, must contain all the parameters as properties.
 * @param {Number} [retryTimeMs] the retry interval in milliseconds - defaultis is 100ms.
 * @param {Number} [timeOutMs] the amount of time to wait until port is free. Default 300ms.
 * @return {Object} A deferred promise from the q library.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.waitUntilFree(44203, 500, 4000)
 * .then(function() {
 *     console.log('Port 44203 is now free.');
 *  }, function(err) {
 *     console.loh('Error: ', error.message);
 *  });
 */
function waitUntilFree(port, retryTimeMs, timeOutMs) {

    // the first arument may be an object, if it is not, make an object
    var opts;
    if (is.obj(port)) {
        opts = port;
        opts.host = '127.0.0.1';
        opts.inUse = false;
    } else {
        opts = makeOptionsObj(port, '127.0.0.1', false, retryTimeMs, timeOutMs);
    }

    return waitForStatus(opts);
}

/**
 * Creates a deferred promise and fulfills it only when the socket is used.
 * Will retry on an interval specified in retryTimeMs.
 * Note: you have to be super user to correctly test system ports (0-1023).
 * @param {Number|Object} port a valid TCP port number. If an object, must contain all the parameters as properties.
 * @param {Number} [retryTimeMs] the retry interval in milliseconds - defaultis is 500ms
 * @param {Number} [timeOutMs] the amount of time to wait until port is free
 * @return {Object} A deferred promise from the q library.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.waitUntilUsedOnHost(44204, 'some.host.com', 500, 4000)
 * .then(function() {
 *     console.log('Port 44204 is now in use.');
 * }, function(err) {
 *     console.log('Error: ', error.message);
 * });
 */
function waitUntilUsedOnHost(port, host, retryTimeMs, timeOutMs) {

    // the first arument may be an object, if it is not, make an object
    var opts;
    if (is.obj(port)) {
        opts = port;
        opts.inUse = true;
    } else {
        opts = makeOptionsObj(port, host, true, retryTimeMs, timeOutMs);
    }

    return waitForStatus(opts);
}

/**
 * For compatibility to previous version of module which did not have support
 * for host addresses. This function works only for localhost.
 * @param {Number} port a valid TCP port number. If an Object, must contain all the parameters as properties.
 * @param {Number} [retryTimeMs] the retry interval in milliseconds - defaultis is 500ms
 * @param {Number} [timeOutMs] the amount of time to wait until port is free
 * @return {Object} A deferred promise from the q library.
 *
 * Example usage:
 *
 * var tcpPortUsed = require('tcp-port-used');
 * tcpPortUsed.waitUntilUsed(44204, 500, 4000)
 * .then(function() {
 *     console.log('Port 44204 is now in use.');
 * }, function(err) {
 *     console.log('Error: ', error.message);
 * });
 */
function waitUntilUsed(port, retryTimeMs, timeOutMs) {

    // the first arument may be an object, if it is not, make an object
    var opts;
    if (is.obj(port)) {
        opts = port;
        opts.host = '127.0.0.1';
        opts.inUse = true;
    } else {
        opts = makeOptionsObj(port, '127.0.0.1', true, retryTimeMs, timeOutMs);
    }

    return waitUntilUsedOnHost(opts);
}

var fetchRetry$1 = function (fetch, defaults) {
  defaults = defaults || {};
  if (typeof fetch !== 'function') {
    throw new ArgumentError('fetch must be a function');
  }

  if (typeof defaults !== 'object') {
    throw new ArgumentError('defaults must be an object');
  }

  if (defaults.retries !== undefined && !isPositiveInteger(defaults.retries)) {
    throw new ArgumentError('retries must be a positive integer');
  }

  if (defaults.retryDelay !== undefined && !isPositiveInteger(defaults.retryDelay) && typeof defaults.retryDelay !== 'function') {
    throw new ArgumentError('retryDelay must be a positive integer or a function returning a positive integer');
  }

  if (defaults.retryOn !== undefined && !Array.isArray(defaults.retryOn) && typeof defaults.retryOn !== 'function') {
    throw new ArgumentError('retryOn property expects an array or function');
  }

  var baseDefaults = {
    retries: 3,
    retryDelay: 1000,
    retryOn: [],
  };

  defaults = Object.assign(baseDefaults, defaults);

  return function fetchRetry(input, init) {
    var retries = defaults.retries;
    var retryDelay = defaults.retryDelay;
    var retryOn = defaults.retryOn;

    if (init && init.retries !== undefined) {
      if (isPositiveInteger(init.retries)) {
        retries = init.retries;
      } else {
        throw new ArgumentError('retries must be a positive integer');
      }
    }

    if (init && init.retryDelay !== undefined) {
      if (isPositiveInteger(init.retryDelay) || (typeof init.retryDelay === 'function')) {
        retryDelay = init.retryDelay;
      } else {
        throw new ArgumentError('retryDelay must be a positive integer or a function returning a positive integer');
      }
    }

    if (init && init.retryOn) {
      if (Array.isArray(init.retryOn) || (typeof init.retryOn === 'function')) {
        retryOn = init.retryOn;
      } else {
        throw new ArgumentError('retryOn property expects an array or function');
      }
    }

    // eslint-disable-next-line no-undef
    return new Promise(function (resolve, reject) {
      var wrappedFetch = function (attempt) {
        // As of node 18, this is no longer needed since node comes with native support for fetch:
        /* istanbul ignore next */
        var _input =
          typeof Request !== 'undefined' && input instanceof Request
            ? input.clone()
            : input;
        fetch(_input, init)
          .then(function (response) {
            if (Array.isArray(retryOn) && retryOn.indexOf(response.status) === -1) {
              resolve(response);
            } else if (typeof retryOn === 'function') {
              try {
                // eslint-disable-next-line no-undef
                return Promise.resolve(retryOn(attempt, null, response))
                  .then(function (retryOnResponse) {
                    if(retryOnResponse) {
                      retry(attempt, null, response);
                    } else {
                      resolve(response);
                    }
                  }).catch(reject);
              } catch (error) {
                reject(error);
              }
            } else {
              if (attempt < retries) {
                retry(attempt, null, response);
              } else {
                resolve(response);
              }
            }
          })
          .catch(function (error) {
            if (typeof retryOn === 'function') {
              try {
                // eslint-disable-next-line no-undef
                Promise.resolve(retryOn(attempt, error, null))
                  .then(function (retryOnResponse) {
                    if(retryOnResponse) {
                      retry(attempt, error, null);
                    } else {
                      reject(error);
                    }
                  })
                  .catch(function(error) {
                    reject(error);
                  });
              } catch(error) {
                reject(error);
              }
            } else if (attempt < retries) {
              retry(attempt, error, null);
            } else {
              reject(error);
            }
          });
      };

      function retry(attempt, error, response) {
        var delay = (typeof retryDelay === 'function') ?
          retryDelay(attempt, error, response) : retryDelay;
        setTimeout(function () {
          wrappedFetch(++attempt);
        }, delay);
      }

      wrappedFetch(0);
    });
  };
};

function isPositiveInteger(value) {
  return Number.isInteger(value) && value >= 0;
}

function ArgumentError(message) {
  this.name = 'ArgumentError';
  this.message = message;
}

var fetchRT = /*@__PURE__*/getDefaultExportFromCjs(fetchRetry$1);

var osutils = {};

var _os = require$$0__default["default"];

osutils.platform = function(){ 
    return process.platform;
};

osutils.cpuCount = function(){ 
    return _os.cpus().length;
};

osutils.sysUptime = function(){ 
    //seconds
    return _os.uptime();
};

osutils.processUptime = function(){ 
    //seconds
    return process.uptime();
};



// Memory
osutils.freemem = function(){
    return _os.freemem() / ( 1024 * 1024 );
};

osutils.totalmem = function(){

    return _os.totalmem() / ( 1024 * 1024 );
};

osutils.freememPercentage = function(){
    return _os.freemem() / _os.totalmem();
};

osutils.freeCommand = function(callback){
    
    // Only Linux
    require$$1__default$3["default"].exec('free -m', function(error, stdout, stderr) {
       
       var lines = stdout.split("\n");
       
       
       var str_mem_info = lines[1].replace( /[\s\n\r]+/g,' ');
       
       var mem_info = str_mem_info.split(' ');
      
       total_mem    = parseFloat(mem_info[1]);
       free_mem     = parseFloat(mem_info[3]);
       buffers_mem  = parseFloat(mem_info[5]);
       cached_mem   = parseFloat(mem_info[6]);
       
       used_mem = total_mem - (free_mem + buffers_mem + cached_mem);
       
       callback(used_mem -2);
    });
};


// Hard Disk Drive
osutils.harddrive = function(callback){
    
    require$$1__default$3["default"].exec('df -k', function(error, stdout, stderr) {
    
        var total = 0;
        var used = 0;
        var free = 0;
    
        var lines = stdout.split("\n");
    
        var str_disk_info = lines[1].replace( /[\s\n\r]+/g,' ');
    
        var disk_info = str_disk_info.split(' ');

        total = Math.ceil((disk_info[1] * 1024)/ Math.pow(1024,2));
        used = Math.ceil(disk_info[2] * 1024 / Math.pow(1024,2)) ;
        free = Math.ceil(disk_info[3] * 1024 / Math.pow(1024,2)) ;

        callback(total, free, used);
    });
};



// Return process running current 
osutils.getProcesses = function(nProcess, callback){
    
    // if nprocess is undefined then is function
    if(typeof nProcess === 'function'){
        
        callback =nProcess; 
        nProcess = 0;
    }   
    
    command = 'ps -eo pcpu,pmem,time,args | sort -k 1 -r | head -n'+10;
    //command = 'ps aux | head -n '+ 11
    //command = 'ps aux | head -n '+ (nProcess + 1)
    if (nProcess > 0)
        command = 'ps -eo pcpu,pmem,time,args | sort -k 1 -r | head -n'+(nProcess + 1);
    
    require$$1__default$3["default"].exec(command, function(error, stdout, stderr) {
        
        var lines = stdout.split("\n");
        lines.shift();
        lines.pop();
       
        var result = '';
        
        
        lines.forEach(function(_item,_i){
            
            var _str = _item.replace( /[\s\n\r]+/g,' ');
            
            _str = _str.split(' ');
            
            // result += _str[10]+" "+_str[9]+" "+_str[2]+" "+_str[3]+"\n";  // process               
            result += _str[1]+" "+_str[2]+" "+_str[3]+" "+_str[4].substring((_str[4].length - 25))+"\n";  // process               
               
        });
        
        callback(result);
    }); 
};



/*
* Returns All the load average usage for 1, 5 or 15 minutes.
*/
osutils.allLoadavg = function(){ 
    
    var loads = _os.loadavg();
    		
    return loads[0].toFixed(4)+','+loads[1].toFixed(4)+','+loads[2].toFixed(4); 
};

/*
* Returns the load average usage for 1, 5 or 15 minutes.
*/
osutils.loadavg = function(_time){ 

    if(_time === undefined || (_time !== 5 && _time !== 15) ) _time = 1;
	
    var loads = _os.loadavg();
    var v = 0;
    if(_time == 1) v = loads[0];
    if(_time == 5) v = loads[1];
    if(_time == 15) v = loads[2];
		
    return v; 
};


osutils.cpuFree = function(callback){ 
    getCPUUsage(callback, true);
};

osutils.cpuUsage = function(callback){ 
    getCPUUsage(callback, false);
};

function getCPUUsage(callback, free){ 
	
    var stats1 = getCPUInfo();
    var startIdle = stats1.idle;
    var startTotal = stats1.total;
	
    setTimeout(function() {
        var stats2 = getCPUInfo();
        var endIdle = stats2.idle;
        var endTotal = stats2.total;
		
        var idle 	= endIdle - startIdle;
        var total 	= endTotal - startTotal;
        var perc	= idle / total;
	  	
        if(free === true)
            callback( perc );
        else
            callback( (1 - perc) );
	  		
    }, 1000 );
}

function getCPUInfo(callback){ 
    var cpus = _os.cpus();
	
    var user = 0;
    var nice = 0;
    var sys = 0;
    var idle = 0;
    var irq = 0;
    var total = 0;
	
    for(var cpu in cpus){
		
        user += cpus[cpu].times.user;
        nice += cpus[cpu].times.nice;
        sys += cpus[cpu].times.sys;
        irq += cpus[cpu].times.irq;
        idle += cpus[cpu].times.idle;
    }
	
    var total = user + nice + sys + idle + irq;
	
    return {
        'idle': idle, 
        'total': total
    };
}

/**
 * Current nitro process
 */
var nitroProcessInfo = undefined;
/**
 * This will retrive GPU informations and persist settings.json
 * Will be called when the extension is loaded to turn on GPU acceleration if supported
 */
function updateNvidiaInfo(nvidiaSettings) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    if (!(process.platform !== "darwin")) return [3 /*break*/, 2];
                    return [4 /*yield*/, Promise.all([
                            updateNvidiaDriverInfo(nvidiaSettings),
                            updateCudaExistence(nvidiaSettings),
                            updateGpuInfo(nvidiaSettings),
                        ])];
                case 1:
                    _a.sent();
                    _a.label = 2;
                case 2: return [2 /*return*/];
            }
        });
    });
}
/**
 * Retrieve current nitro process
 */
var getNitroProcessInfo = function (subprocess) {
    nitroProcessInfo = {
        isRunning: subprocess != null,
    };
    return nitroProcessInfo;
};
/**
 * Validate nvidia and cuda for linux and windows
 */
function updateNvidiaDriverInfo(nvidiaSettings) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            node_child_process.exec("nvidia-smi --query-gpu=driver_version --format=csv,noheader", function (error, stdout) {
                if (!error) {
                    var firstLine = stdout.split("\n")[0].trim();
                    nvidiaSettings["nvidia_driver"].exist = true;
                    nvidiaSettings["nvidia_driver"].version = firstLine;
                }
                else {
                    nvidiaSettings["nvidia_driver"].exist = false;
                }
            });
            return [2 /*return*/];
        });
    });
}
/**
 * Check if file exists in paths
 */
function checkFileExistenceInPaths(file, paths) {
    return paths.some(function (p) { return fs.existsSync(path__default["default"].join(p, file)); });
}
/**
 * Validate cuda for linux and windows
 */
function updateCudaExistence(nvidiaSettings) {
    var filesCuda12;
    var filesCuda11;
    var paths;
    var cudaVersion = "";
    if (process.platform === "win32") {
        filesCuda12 = ["cublas64_12.dll", "cudart64_12.dll", "cublasLt64_12.dll"];
        filesCuda11 = ["cublas64_11.dll", "cudart64_11.dll", "cublasLt64_11.dll"];
        paths = process.env.PATH ? process.env.PATH.split(path__default["default"].delimiter) : [];
    }
    else {
        filesCuda12 = ["libcudart.so.12", "libcublas.so.12", "libcublasLt.so.12"];
        filesCuda11 = ["libcudart.so.11.0", "libcublas.so.11", "libcublasLt.so.11"];
        paths = process.env.LD_LIBRARY_PATH
            ? process.env.LD_LIBRARY_PATH.split(path__default["default"].delimiter)
            : [];
        paths.push("/usr/lib/x86_64-linux-gnu/");
    }
    var cudaExists = filesCuda12.every(function (file) { return fs.existsSync(file) || checkFileExistenceInPaths(file, paths); });
    if (!cudaExists) {
        cudaExists = filesCuda11.every(function (file) { return fs.existsSync(file) || checkFileExistenceInPaths(file, paths); });
        if (cudaExists) {
            cudaVersion = "11";
        }
    }
    else {
        cudaVersion = "12";
    }
    nvidiaSettings["cuda"].exist = cudaExists;
    nvidiaSettings["cuda"].version = cudaVersion;
    if (cudaExists) {
        nvidiaSettings.run_mode = "gpu";
    }
}
/**
 * Get GPU information
 */
function updateGpuInfo(nvidiaSettings) {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            node_child_process.exec("nvidia-smi --query-gpu=index,memory.total --format=csv,noheader,nounits", function (error, stdout) {
                if (!error) {
                    // Get GPU info and gpu has higher memory first
                    var highestVram_1 = 0;
                    var highestVramId_1 = "0";
                    var gpus = stdout
                        .trim()
                        .split("\n")
                        .map(function (line) {
                        var _a = line.split(", "), id = _a[0], vram = _a[1];
                        vram = vram.replace(/\r/g, "");
                        if (parseFloat(vram) > highestVram_1) {
                            highestVram_1 = parseFloat(vram);
                            highestVramId_1 = id;
                        }
                        return { id: id, vram: vram };
                    });
                    nvidiaSettings["gpus"] = gpus;
                    nvidiaSettings["gpu_highest_vram"] = highestVramId_1;
                }
                else {
                    nvidiaSettings["gpus"] = [];
                }
            });
            return [2 /*return*/];
        });
    });
}

/**
 * Find which executable file to run based on the current platform.
 * @returns The name of the executable file to run.
 */
var executableNitroFile = function (nvidiaSettings) {
    var binaryFolder = path__default["default"].join(__dirname, "..", "bin"); // Current directory by default
    var cudaVisibleDevices = "";
    var binaryName = "nitro";
    /**
     * The binary folder is different for each platform.
     */
    if (process.platform === "win32") {
        /**
         *  For Windows: win-cpu, win-cuda-11-7, win-cuda-12-0
         */
        if (nvidiaSettings["run_mode"] === "cpu") {
            binaryFolder = path__default["default"].join(binaryFolder, "win-cpu");
        }
        else {
            if (nvidiaSettings["cuda"].version === "12") {
                binaryFolder = path__default["default"].join(binaryFolder, "win-cuda-12-0");
            }
            else {
                binaryFolder = path__default["default"].join(binaryFolder, "win-cuda-11-7");
            }
            cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
        }
        binaryName = "nitro.exe";
    }
    else if (process.platform === "darwin") {
        /**
         *  For MacOS: mac-arm64 (Silicon), mac-x64 (InteL)
         */
        if (process.arch === "arm64") {
            binaryFolder = path__default["default"].join(binaryFolder, "mac-arm64");
        }
        else {
            binaryFolder = path__default["default"].join(binaryFolder, "mac-x64");
        }
    }
    else {
        if (nvidiaSettings["run_mode"] === "cpu") {
            binaryFolder = path__default["default"].join(binaryFolder, "linux-cpu");
        }
        else {
            if (nvidiaSettings["cuda"].version === "12") {
                binaryFolder = path__default["default"].join(binaryFolder, "linux-cuda-12-0");
            }
            else {
                binaryFolder = path__default["default"].join(binaryFolder, "linux-cuda-11-7");
            }
            cudaVisibleDevices = nvidiaSettings["gpu_highest_vram"];
        }
    }
    return {
        executablePath: path__default["default"].join(binaryFolder, binaryName),
        cudaVisibleDevices: cudaVisibleDevices,
    };
};

// Polyfill fetch with retry
var fetchRetry = fetchRT(fetch);
// The PORT to use for the Nitro subprocess
var PORT = 3928;
// The HOST address to use for the Nitro subprocess
var LOCAL_HOST = "127.0.0.1";
// The URL for the Nitro subprocess
var NITRO_HTTP_SERVER_URL = "http://".concat(LOCAL_HOST, ":").concat(PORT);
// The URL for the Nitro subprocess to load a model
var NITRO_HTTP_LOAD_MODEL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/loadmodel");
// The URL for the Nitro subprocess to validate a model
var NITRO_HTTP_VALIDATE_MODEL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/modelstatus");
// The URL for the Nitro subprocess to kill itself
var NITRO_HTTP_KILL_URL = "".concat(NITRO_HTTP_SERVER_URL, "/processmanager/destroy");
// The URL for the Nitro subprocess to run chat completion
var NITRO_HTTP_CHAT_URL = "".concat(NITRO_HTTP_SERVER_URL, "/inferences/llamacpp/chat_completion");
// The default config for using Nvidia GPU
var NVIDIA_DEFAULT_CONFIG = {
    notify: true,
    run_mode: "cpu",
    nvidia_driver: {
        exist: false,
        version: "",
    },
    cuda: {
        exist: false,
        version: "",
    },
    gpus: [],
    gpu_highest_vram: "",
};
// The supported model format
// TODO: Should be an array to support more models
var SUPPORTED_MODEL_FORMATS = [".gguf"];
// The subprocess instance for Nitro
var subprocess = undefined;
// The current model file url
var currentModelFile = "";
// The current model settings
var currentSettings = undefined;
// The Nvidia info file for checking for CUDA support on the system
var nvidiaConfig = NVIDIA_DEFAULT_CONFIG;
// The logger to use, default to stdout
var log = function (message) {
    return process.stdout.write(message + os__default["default"].EOL);
};
/**
 * Get current Nvidia config
 * @returns {NitroNvidiaConfig} A copy of the config object
 * The returned object should be used for reading only
 * Writing to config should be via the function {@setNvidiaConfig}
 */
function getNvidiaConfig() {
    return Object.assign({}, nvidiaConfig);
}
/**
 * Set custom Nvidia config for running inference over GPU
 * @param {NitroNvidiaConfig} config The new config to apply
 */
function setNvidiaConfig(config) {
    nvidiaConfig = config;
}
/**
 * Set logger before running nitro
 * @param {NitroLogger} logger The logger to use
 */
function setLogger(logger) {
    log = logger;
}
/**
 * Stops a Nitro subprocess.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
function stopModel() {
    return killSubprocess();
}
/**
 * Initializes a Nitro subprocess to load a machine learning model.
 * @param modelFullPath - The absolute full path to model directory.
 * @param wrapper - The model wrapper.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 * TODO: Should pass absolute of the model file instead of just the name - So we can modurize the module.ts to npm package
 */
function runModel(_a) {
    var modelFullPath = _a.modelFullPath, promptTemplate = _a.promptTemplate;
    return __awaiter(this, void 0, void 0, function () {
        var files, ggufBinFile, nitroResourceProbe, prompt;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    files = fs__default["default"].readdirSync(modelFullPath);
                    ggufBinFile = files.find(function (file) {
                        return file === path__default["default"].basename(modelFullPath) ||
                            SUPPORTED_MODEL_FORMATS.some(function (ext) { return file.toLowerCase().endsWith(ext); });
                    });
                    if (!ggufBinFile)
                        return [2 /*return*/, Promise.reject("No GGUF model file found")];
                    currentModelFile = path__default["default"].join(modelFullPath, ggufBinFile);
                    return [4 /*yield*/, getResourcesInfo()];
                case 1:
                    nitroResourceProbe = _b.sent();
                    prompt = {};
                    if (promptTemplate) {
                        try {
                            Object.assign(prompt, promptTemplateConverter(promptTemplate));
                        }
                        catch (e) {
                            return [2 /*return*/, Promise.reject(e)];
                        }
                    }
                    currentSettings = __assign(__assign({}, prompt), { llama_model_path: currentModelFile, 
                        // This is critical and requires real system information
                        cpu_threads: Math.max(1, Math.round(nitroResourceProbe.numCpuPhysicalCore / 2)) });
                    return [2 /*return*/, runNitroAndLoadModel()];
            }
        });
    });
}
/**
 * 1. Spawn Nitro process
 * 2. Load model into Nitro subprocess
 * 3. Validate model status
 * @returns
 */
function runNitroAndLoadModel() {
    return __awaiter(this, void 0, void 0, function () {
        return __generator(this, function (_a) {
            // Gather system information for CPU physical cores and memory
            return [2 /*return*/, killSubprocess()
                    .then(function () { return tcpPortUsed.waitUntilFree(PORT, 300, 5000); })
                    .then(function () {
                    /**
                     * There is a problem with Windows process manager
                     * Should wait for awhile to make sure the port is free and subprocess is killed
                     * The tested threshold is 500ms
                     **/
                    if (process.platform === "win32") {
                        return new Promise(function (resolve) { return setTimeout(function () { return resolve({}); }, 500); });
                    }
                    else {
                        return Promise.resolve({});
                    }
                })
                    .then(spawnNitroProcess)
                    .then(function () { return loadLLMModel(currentSettings); })
                    .then(validateModelStatus)
                    .catch(function (err) {
                    // TODO: Broadcast error so app could display proper error message
                    log("[NITRO]::Error: ".concat(err));
                    return { error: err };
                })];
        });
    });
}
/**
 * Parse prompt template into agrs settings
 * @param {string} promptTemplate Template as string
 * @returns {(NitroPromptSetting | never)} parsed prompt setting
 * @throws {Error} if cannot split promptTemplate
 */
function promptTemplateConverter(promptTemplate) {
    // Split the string using the markers
    var systemMarker = "{system_message}";
    var promptMarker = "{prompt}";
    if (promptTemplate.includes(systemMarker) &&
        promptTemplate.includes(promptMarker)) {
        // Find the indices of the markers
        var systemIndex = promptTemplate.indexOf(systemMarker);
        var promptIndex = promptTemplate.indexOf(promptMarker);
        // Extract the parts of the string
        var system_prompt = promptTemplate.substring(0, systemIndex);
        var user_prompt = promptTemplate.substring(systemIndex + systemMarker.length, promptIndex);
        var ai_prompt = promptTemplate.substring(promptIndex + promptMarker.length);
        // Return the split parts
        return { system_prompt: system_prompt, user_prompt: user_prompt, ai_prompt: ai_prompt };
    }
    else if (promptTemplate.includes(promptMarker)) {
        // Extract the parts of the string for the case where only promptMarker is present
        var promptIndex = promptTemplate.indexOf(promptMarker);
        var user_prompt = promptTemplate.substring(0, promptIndex);
        var ai_prompt = promptTemplate.substring(promptIndex + promptMarker.length);
        // Return the split parts
        return { user_prompt: user_prompt, ai_prompt: ai_prompt };
    }
    // Throw error if none of the conditions are met
    throw Error("Cannot split prompt template");
}
/**
 * Loads a LLM model into the Nitro subprocess by sending a HTTP POST request.
 * @returns A Promise that resolves when the model is loaded successfully, or rejects with an error message if the model is not found or fails to load.
 */
function loadLLMModel(settings) {
    return __awaiter(this, void 0, void 0, function () {
        var res, err_1;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    log("[NITRO]::Debug: Loading model with params ".concat(JSON.stringify(settings)));
                    _a.label = 1;
                case 1:
                    _a.trys.push([1, 4, , 6]);
                    return [4 /*yield*/, fetchRetry(NITRO_HTTP_LOAD_MODEL_URL, {
                            method: "POST",
                            headers: {
                                "Content-Type": "application/json",
                            },
                            body: JSON.stringify(settings),
                            retries: 3,
                            retryDelay: 500,
                        })];
                case 2:
                    res = _a.sent();
                    // FIXME: Actually check response, as the model directory might not exist
                    log("[NITRO]::Debug: Load model success with response ".concat(JSON.stringify(res)));
                    return [4 /*yield*/, Promise.resolve(res)];
                case 3: return [2 /*return*/, _a.sent()];
                case 4:
                    err_1 = _a.sent();
                    log("[NITRO]::Error: Load model failed with error ".concat(err_1));
                    return [4 /*yield*/, Promise.reject()];
                case 5: return [2 /*return*/, _a.sent()];
                case 6: return [2 /*return*/];
            }
        });
    });
}
/**
 * Run chat completion by sending a HTTP POST request and stream the response if outStream is specified
 * @param {any} request The request that is then sent to nitro
 * @param {WritableStream} outStream Optional stream that consume the response body
 * @returns {Promise<Response>} A Promise that resolves when the chat completion success, or rejects with an error if the completion fails.
 * @description If outStream is specified, the response body is consumed and cannot be used to reconstruct the data
 */
function chatCompletion(request, outStream) {
    return __awaiter(this, void 0, void 0, function () {
        var _this = this;
        return __generator(this, function (_a) {
            if (outStream) {
                // Add stream option if there is an outStream specified when calling this function
                Object.assign(request, {
                    stream: true,
                });
            }
            log("[NITRO]::Debug: Running chat completion with request ".concat(JSON.stringify(request)));
            return [2 /*return*/, fetchRetry(NITRO_HTTP_CHAT_URL, {
                    method: "POST",
                    headers: {
                        "Content-Type": "application/json",
                        Accept: "text/event-stream",
                        "Access-Control-Allow-Origin": "*",
                    },
                    body: JSON.stringify(request),
                    retries: 3,
                    retryDelay: 500,
                })
                    .then(function (response) { return __awaiter(_this, void 0, void 0, function () {
                    var outPipe;
                    return __generator(this, function (_a) {
                        switch (_a.label) {
                            case 0:
                                if (!outStream) return [3 /*break*/, 2];
                                if (!response.body) {
                                    throw new Error("Error running chat completion");
                                }
                                outPipe = response.body
                                    .pipeThrough(new TextDecoderStream())
                                    .pipeTo(outStream);
                                // Wait for all the streams to complete before returning from async function
                                return [4 /*yield*/, outPipe];
                            case 1:
                                // Wait for all the streams to complete before returning from async function
                                _a.sent();
                                _a.label = 2;
                            case 2:
                                log("[NITRO]::Debug: Chat completion success");
                                return [2 /*return*/, response];
                        }
                    });
                }); })
                    .catch(function (err) {
                    log("[NITRO]::Error: Chat completion failed with error ".concat(err));
                    throw err;
                })];
        });
    });
}
/**
 * Validates the status of a model.
 * @returns {Promise<NitroModelOperationResponse>} A promise that resolves to an object.
 * If the model is loaded successfully, the object is empty.
 * If the model is not loaded successfully, the object contains an error message.
 */
function validateModelStatus() {
    return __awaiter(this, void 0, void 0, function () {
        var _this = this;
        return __generator(this, function (_a) {
            // Send a GET request to the validation URL.
            // Retry the request up to 3 times if it fails, with a delay of 500 milliseconds between retries.
            return [2 /*return*/, fetchRetry(NITRO_HTTP_VALIDATE_MODEL_URL, {
                    method: "GET",
                    headers: {
                        "Content-Type": "application/json",
                    },
                    retries: 5,
                    retryDelay: 500,
                }).then(function (res) { return __awaiter(_this, void 0, void 0, function () {
                    var body;
                    return __generator(this, function (_a) {
                        switch (_a.label) {
                            case 0:
                                log("[NITRO]::Debug: Validate model state success with response ".concat(JSON.stringify(res)));
                                if (!res.ok) return [3 /*break*/, 2];
                                return [4 /*yield*/, res.json()];
                            case 1:
                                body = _a.sent();
                                // If the model is loaded, return an empty object.
                                // Otherwise, return an object with an error message.
                                if (body.model_loaded) {
                                    return [2 /*return*/, Promise.resolve({})];
                                }
                                _a.label = 2;
                            case 2: return [2 /*return*/, Promise.resolve({ error: "Validate model status failed" })];
                        }
                    });
                }); })];
        });
    });
}
/**
 * Terminates the Nitro subprocess.
 * @returns A Promise that resolves when the subprocess is terminated successfully, or rejects with an error message if the subprocess fails to terminate.
 */
function killSubprocess() {
    return __awaiter(this, void 0, void 0, function () {
        var controller;
        return __generator(this, function (_a) {
            controller = new AbortController();
            setTimeout(function () { return controller.abort(); }, 5000);
            log("[NITRO]::Debug: Request to kill Nitro");
            return [2 /*return*/, fetch(NITRO_HTTP_KILL_URL, {
                    method: "DELETE",
                    signal: controller.signal,
                })
                    .then(function () {
                    subprocess === null || subprocess === void 0 ? void 0 : subprocess.kill();
                    subprocess = undefined;
                })
                    .catch(function (err) { return ({ error: err }); })
                    .then(function () { return tcpPortUsed.waitUntilFree(PORT, 300, 5000); })
                    .then(function () { return log("[NITRO]::Debug: Nitro process is terminated"); })
                    .then(function () { return Promise.resolve({}); })];
        });
    });
}
/**
 * Spawns a Nitro subprocess.
 * @returns A promise that resolves when the Nitro subprocess is started.
 */
function spawnNitroProcess() {
    var _this = this;
    log("[NITRO]::Debug: Spawning Nitro subprocess...");
    return new Promise(function (resolve, reject) { return __awaiter(_this, void 0, void 0, function () {
        var binaryFolder, executableOptions, args;
        return __generator(this, function (_a) {
            binaryFolder = path__default["default"].join(__dirname, "..", "bin");
            executableOptions = executableNitroFile(nvidiaConfig);
            args = ["1", LOCAL_HOST, PORT.toString()];
            // Execute the binary
            log("[NITRO]::Debug: Spawn nitro at path: ".concat(executableOptions.executablePath, ", and args: ").concat(args));
            subprocess = node_child_process.spawn(executableOptions.executablePath, ["1", LOCAL_HOST, PORT.toString()], {
                cwd: binaryFolder,
                env: __assign(__assign({}, process.env), { CUDA_VISIBLE_DEVICES: executableOptions.cudaVisibleDevices }),
            });
            // Handle subprocess output
            subprocess.stdout.on("data", function (data) {
                log("[NITRO]::Debug: ".concat(data));
            });
            subprocess.stderr.on("data", function (data) {
                log("[NITRO]::Error: ".concat(data));
            });
            subprocess.on("close", function (code) {
                log("[NITRO]::Debug: Nitro exited with code: ".concat(code));
                subprocess = undefined;
                reject("child process exited with code ".concat(code));
            });
            tcpPortUsed.waitUntilUsed(PORT, 300, 30000).then(function () {
                log("[NITRO]::Debug: Nitro is ready");
                resolve({});
            });
            return [2 /*return*/];
        });
    }); });
}
/**
 * Get the system resources information
 */
function getResourcesInfo() {
    var _this = this;
    return new Promise(function (resolve) { return __awaiter(_this, void 0, void 0, function () {
        var cpu, response;
        return __generator(this, function (_a) {
            cpu = osutils.cpuCount();
            log("[NITRO]::CPU informations - ".concat(cpu));
            response = {
                numCpuPhysicalCore: cpu,
                memAvailable: 0,
            };
            resolve(response);
            return [2 /*return*/];
        });
    }); });
}
var index = {
    getNvidiaConfig: getNvidiaConfig,
    setNvidiaConfig: setNvidiaConfig,
    setLogger: setLogger,
    runModel: runModel,
    stopModel: stopModel,
    loadLLMModel: loadLLMModel,
    validateModelStatus: validateModelStatus,
    chatCompletion: chatCompletion,
    killSubprocess: killSubprocess,
    updateNvidiaInfo: function () { return __awaiter(void 0, void 0, void 0, function () { return __generator(this, function (_a) {
        switch (_a.label) {
            case 0: return [4 /*yield*/, updateNvidiaInfo(nvidiaConfig)];
            case 1: return [2 /*return*/, _a.sent()];
        }
    }); }); },
    getCurrentNitroProcessInfo: function () { return getNitroProcessInfo(subprocess); },
};

module.exports = index;
//# sourceMappingURL=index.cjs.js.map
