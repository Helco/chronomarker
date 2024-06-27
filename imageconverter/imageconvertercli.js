var imageconverter = require("./imageconverter");
var canvas = require("canvas");

(async () => {
    const img = await canvas.loadImage("../chronomarker-watch/starfield-planet.png");
    const str = await imageconverter.imagetoString(img, {
        mode: "2bitbw",
        transparent: true,
        compression: true,
        output: "string"
    });
    console.log(str);
})();
