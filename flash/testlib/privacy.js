// SPDX-License-Identifier: LGPL-2.1-or-later
// What must never be in a published file (the idea of JukeDial's tools/web_publish.py scan). Used by check.mjs and tested in test/privacy.test.js.
export const PRIVATE = [
  ["a home-directory path", /\/home\/[A-Za-z0-9_.-]+\/|\/Users\/[A-Za-z0-9_.-]+\/|\/media\/[A-Za-z0-9_.-]+\/|[A-Za-z]:\\{1,2}Users\\|[A-Za-z]:\/Users\//],
  ["a MAC address", /(?<![0-9A-Fa-f:])(?!00(?::00){5}|ff(?::ff){5}|FF(?::FF){5})[0-9A-Fa-f]{2}(?::[0-9A-Fa-f]{2}){5}(?![0-9A-Fa-f:])/],
  ["a USB serial / by-id path", /usb-[A-Za-z0-9]+_[A-Za-z0-9_.-]+_[A-Za-z0-9:]{8,}-if\d\d|\/dev\/serial\/by-id|\/dev\/tty(?:ACM|USB)\d|COM\d+\b.*\bserial\b/],
  ["a private key marker", /-----BEGIN [A-Z ]*PRIVATE KEY|ssh-rsa AAAA|AKIA[0-9A-Z]{16}/],
  ["a private IP address", /(?<![\d.])(?:192\.168\.\d{1,3}\.\d{1,3}|10\.\d{1,3}\.\d{1,3}\.\d{1,3}|172\.(?:1[6-9]|2\d|3[01])\.\d{1,3}\.\d{1,3}|169\.254\.\d{1,3}\.\d{1,3})(?![\d.])/],
];
