if (typeof Dickens !== 'object') require('dickens-polyfill.js');
if (settings.log) log('Running starfield.app.js');

var notifyT = 0;
var nextValue = 0;

function setStarfieldBLEServices() {
  if (notifyT != 0) clearInterval(notifyT);
  let svc = {
    ancs: false,
    ams: false,
    cts: false,
    uart: true,
    advertise: [0x7201]
  };
  NRF.disconnect();
  NRF.setServices({
    0x7201: {
      0xC520: {
        writable: true,
        notify: true,
        maxLen: 128,
        value: [(nextValue++ % 256)],
        onWrite: function (evt) {
          g.drawSlice(evt.data[0], evt.data[0] + 0.3, 72, 95);
          g.setColor(0).fillRect(100, 100, 140, 140).setColor("#fff");
          g.drawString("" + evt.data.length, 101, 101, true);
        }
      }
    }
  }, svc);
  NRF.setAdvertising({}, {
    showName: false,
    manufacturer: 0x7201,
    manufacturerData: [1]
  });
  notifyT = setInterval(() => {
    try {
      NRF.updateServices({
        0x7201: {
          0xC520: {
            value: [(nextValue++ % 256)],
            notify: true
          }
        }
      });
    }
    catch(e) {}
  }, 3000);
  console.log("set new services");
}

setStarfieldBLEServices();
Bangle.setLCDTimeout(0);

const maxO2CO2 = 63;
const o2StartAngle = 1.5 * 3.141592653;
const co2StartAngle = 0.5 * 3.141592653;
function drawO2CO2(o2, co2) {
  if (o2 > 0 && co2 > 0)
    o2--, co2--;
  if (o2 > 0)
    g.setColor("#fff").drawSlice(o2StartAngle - 3.141592653 * o2 / maxO2CO2, o2StartAngle, 74, 95);
  if (co2 > 0)
    g.setColor("#f00").drawSlice(co2StartAngle, co2StartAngle + 3.141592653 * co2 / maxO2CO2, 74, 95);
}

const imagePlanet = require("heatshrink").decompress(atob("slk4UA///ttrAYP33oVJgWqAAOAFqQWCAAWoC6EqCgIEBiwZRhWq0AFCgofCJp2q1dVAgMVAYX6GRsKBwIVBAANADgUqGRgNJgsCGRYMKMwQyKlWlIwIKGipWDPRPVMAowCqDoCJJKqDgI0Dgo4DtRLIhQJFGoQ2EjwnDMQrTNihkHUBgACgiXHhSTCqgYLhRLGEAR5CDRMUIQ8oAgaUEDA0ADAwGGirJFAALKBRopRHYgQaEgoEBX4rFGJwotBAgIIBhwrEYpbjDDYIcBLompBgQaJipNDgPqbwetBQJcFABVKDAZPDLALhLAANaO4R7GLYgAIqQYDypwFYwIZKiRGClRECJIIUDNBUQSwUrJIoUCDwIZIiEoAYKyEawgEBQJEQbgTkEAAMFFwcVDI8QhStFbIgzDQA9QVYMCVooZDCgRmHqAWBYwwZECgRRBOIlAJAJMBH4JZGQAkFBgcVPQUKBwLgFCgYGDdASDD0EqYIo0FEAhNBHIYYBFgj6FJgoAF1DJCfRBMGDAr7CPIpMGDA8olBSCEwgsFGRAYBEYKXCJhLMDDAmpBAaUEbYzUGlGlU5MVIwgyGlWkSgwEDFggyGhWgDAp0EgplFDBgyLKIgYIGQgsFMogYDXQoyDFgoFEDALgCIwgyEPAo4DDAIFCioPEZYgYEgJLCDALBISYYSDEYsK0IKEqoUDJZkKfIsFCAYDDS4o8CDAwQEAYbjFVAQYHI4ZgDWAojChWkcAxHDCgYYFToIYII4YcDiiwGhWsDoNVVg5kELI0q1AOBfIhcDDgYYFKIMolBVCRQhHCPoiWFigYBBIZLDLgaaGKIcolIJDCgZgDDBMBlEqBIZcHEATIGgOoDAh5HJwwYJPI5OGAAeghQYEIYYYPwIYDMARrEDBeAe4gDCDBkCwEC2BfCDCmgLAYYJVwxIBgGsYg4YGcAsKAQOrHgY1CPgwYGlACBlarDVxIYFgvoAYMqoBLBLAYRDD4RYDAANX0BNC6EBqtVMwwYDGAlSDAe0A4KxHKgpSCVoKxC0rqFCIZRCDAlUVoQAB0KHFGoYYCYwZuBVoQAB1ASDbAh/CSgicBlQSDlUVBgaNDCoR7EFIOocgmFGQUBP4zJCD4UCSgR9C0CuBAAJbDPAQfDDgNaPYZkCgAXFPARpDAwMBtSoFNIhCENI0VMQgYC2AHFL4TFDAwNeDA0KcQxfBJIlUgNKPYh9C1LJEWoS4DJoMFlR7FJYXVFIUFGASTFqxJGDAWAWAYzEJoUBrRJGJYQiCchCTBJJAAB1XVVodVGAZ6Bq2qC5BLB1r8HitQitqJJBLC1QXCJAgwDJJIyCZQpNBGQNKSZAyFF4ZlCGQIwMGQWvMokFq+qGBgyD1YZDC4IwOGQQSBDQPqAgIwODIoACC6IaECxYA=="));

function drawMoon (phase) {
  var x = 119,
    y = 119,
    r0 = 50,
      tilt = 1;
  var r1, r2
  if (phase < 0.5) {
    r1 = r0
    r2 = -r0 + phase * 4 * r0
  } else {
    r1 = r0
    r2 = r0 - (phase - 0.5) * 4 * r0
    tilt += 3.141592653;
  }
  var i, s, c
  var inc = 3.142 / 15
  var crater = []
  var ci = 0
  var poly = []
  var sT = Math.cos(tilt),
    cT = Math.sin(tilt)
  for (i = 0; i < 3.142; i += inc) {
    s = Math.sin(i)
    c = Math.cos(i)
    poly.push(x - r1 * c * sT + r2 * s * cT, y + r1 * c * cT + r2 * s * sT)
  }
  for (; i < 6.283; i += inc) {
    s = Math.sin(i)
    c = Math.cos(i)
    poly.push(x - r1 * c * sT + r1 * s * cT, y + r1 * c * cT + r1 * s * sT)
  }
  if (r2 - r1 < 2 * r0) {
    g.setColor("#ddd").fillPolyAA(poly)
  }
 
}

function fullDraw(o2, co2) {
  g.setBgColor("#111");
  g.clear(false);
  g.drawBackground();
  g.drawTicks(0.3);
  drawO2CO2(o2, co2);
  g.setFontArchitekt15(1);
  g.setColor("#fff");
  g.drawString("O2", 120 - 94, 100);
  g.drawString("CO2", 120 + 69, 100);
  
  drawMoon(o2 / 63);
  g.drawImage(imagePlanet, 70, 70);
}

var i = 256;
setInterval(() => {
  i++;
  var o2 = 0, co2 = 0;
  var i1 = i % 64;
  var i2 = (i / 64)|0;
  if (i2 == 0) o2 = i1;
  if (i2 == 1) o2 = 63 - i1;
  if (i2 == 2) co2 = i1;
  if (i2 == 3) co2 = 63 - i1;
  if (i2 == 4) { o2 = i1 / 2; co2 = i1 / 2; }
  if (i2 == 5) { i1 = 63 - i1; o2 = i1 / 2; co2 = i1 / 2; }
  if (i2 == 6) { o2 = 63 - i1; co2 = i1; }
  if (i2 == 7) { co2 = 63 - i1; o2 = i1; }
  if (i2 >= 8) i = 256;
  
  fullDraw(o2, co2);
}, 55);


function exitTo(nextApp) {
  if (notifyT != 0) clearInterval(notifyT);
  NRF.disconnect();
  Dickens.setBLEServices();
  Bangle.setLCDTimeout(10); // just paranoia, clock should set it to the default
  load(nextApp);
}

setWatch(_ => exitTo('clock.app.js'), BTN1, { edge: 1, repeat: true });
setWatch(_ => exitTo('clock.app.js'), BTN2, { edge: 1 });
setWatch(_ => exitTo('clock.app.js'), BTN4, { edge: 1 });
setWatch(_ => setStarfieldBLEServices(), BTN3, { edge: 1 });