# ImageConverter

This is a quick and dirty Node.JS adaptation of the image conversion webtool found at https://github.com/espruino/EspruinoWebTools.

Neccessary changes were:
  - Loading heatshrink wrapper module using `require`
  - Creating heatshrink wrapper module with expected API and compression config
  - Asyncifying functions because node-heatshrink is async as well.
