// SPDX-License-Identifier: LGPL-2.1-or-later
import test from "node:test";
import assert from "node:assert/strict";
import { PRIVATE } from "../testlib/privacy.js";

const hit = (s) => PRIVATE.filter(([, rx]) => rx.test(s)).map(([w]) => w);
test("the privacy scan catches home paths, MACs, USB serials, key markers, private IPs", () => {
  assert.deepEqual(hit("see /home/alice/proj/x.c"), ["a home-directory path"]);
  assert.deepEqual(hit("C:\\Users\\bob\\x"), ["a home-directory path"]);
  assert.deepEqual(hit("/Users/carol/x"), ["a home-directory path"]);
  assert.deepEqual(hit("mac 02:AB:CD:12:34:56 end"), ["a MAC address"]);
  assert.deepEqual(hit("/dev/serial/by-id/usb-1a86_USB_Serial_5A7B012345-if00"), ["a USB serial / by-id path"]);
  assert.deepEqual(hit("-----BEGIN OPENSSH PRIVATE KEY-----"), ["a private key marker"]);
  for (const ip of ["192.168.1.20", "10.0.0.7", "172.16.5.4", "172.31.255.1", "169.254.1.1"]) assert.deepEqual(hit("host " + ip), ["a private IP address"], ip);
});
test("and leaves harmless text alone", () => {
  for (const s of ["esptool-js 0.7.1", "00:00:00:00:00:00", "ff:ff:ff:ff:ff:ff", "172.32.0.1", "8.8.8.8", "version 10.2.3", "usb DAC", "/home/ and /Users/", "https://github.com/JokerCatz/NightStandNano"]) assert.deepEqual(hit(s), [], s);
});
