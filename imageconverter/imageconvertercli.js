var imageconverter = require("./imageconverter");
var canvas = require("canvas");

(async () => {
    const img = await canvas.loadImage("../chronomarker-watch/starfield-icons12.png");
    const str = await imageconverter.imagetoString(img, {
        mode: "opt1bit",
        transparent: true,
        compression: false,
        output: "string"
    });
    console.log(str);
})();
