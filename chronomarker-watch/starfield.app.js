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

const imagePlanet = require("heatshrink").decompress(atob("slkwUAABMD/4AB4APKAA4WCAAX8C6E/CgIEBlYZRh//+AFChQfCJpwqB1QEBlQDCGR0PBwIVBAAOADgU/GRgNJhUDGRYMKMwQyKn/6IwIKGlRWDPRPqMAowC0DoCJJKqDgQ0DhQ4D35LIh4JFGoQ2El4nDMQrTNlBkHUBgAChCXHh6TC1AYLh5LGEAR5CDRMoIQ88AgaUEDA0ADAwGGlTJFAALKBRopRHYgQaEhQEBX4rFGJwotBAgIIBhYrEYpbjDDYIcBLon+BgQaJlRNDgX/bwf+BQJcFABWPDAZPDLALhLAAOvO4R7GLYgAI0YYD9RwFYwIZKkZGCn5ECJIIUDNBUgSwStFCgYeBDJEgngDBWQjWEAgKBIkAuCcggABhQuDlQZHkEPVorZEGYaAH0CrBgatFDIYUCMw+gCwLGGDIgUCKIJxEwBIBJgI/BLIyAEhQMDlR6Ch4OBcAoUDAwboCQYfwn7BFGgogEJoI5DDAIsEfQpMFAAv8fAr6FJgwYFfYR5FJgwYHnk8KQQmEFgoyIDAIjBS4RMJZgYYE/wIDSgjbGag08/SnJlRGEGQ0//CUGAgYsEGQ0P+AYFOgkKMooYMGRZREDBAyEFgplEDAa6FGQYsFAogYBcARGEGQh4FHAYYBAoUqB4jLEDAkCJYQYBYJCTDCQYjFh/yBQmqCgZLMh75FhQQDAYaXFHgQYGCAgDDcYqoCDA5HDMAawFEYUP/DgGI4YUDDAqdBDBBHDDgcoWA0P/gdB1SsHMghZGn4YBlT5ELgYcDDApRBnk8KoSKEI4R9ESwsoDAIJDJYZcDTQxRDnk+BIYUDMAYYJgU8n4JDLg4gCZA0C/gYEPI5OGDBJ5HJwwAD+EPDAhDDDB/CDAZgCNYgYL4D3EAYQYMgfAgfwL4QYVLAYYJVwxIBgH4Yg4YGcAsPAQP+HgY1CPgwYGngCBn6rDVxIYFhX8DAWAJYJYDCIYfCLAYAB1fwJoXggWq1RmGDAYwE0YYD/AHBWI5UFKQStBWIX6dQoRDKIQYE1CtCAAPyQ4o1DDATGDNwKtCAAP8CQbYEP4SUETgKqBAAU/lQMDRoYVCPYgpBVoR9C5QyCgR/GZIQfCgaUCPoXwVwIABLYZ4CD4YcB157DMgUAC4p4CNIYGBgW/VAppEIQhpGlRiEDAXgA4pfCYoYGB1YYGh7iGL4JJE1ECx57EPoX+ZIi1CXAZNBhU/PYpLC9QpChQwCSYpJHDAXAWAYzEJoUC15JGJYQiCchCTBJJAAB//qVoeqGAZ6B1f/C5BLB/z8HlWglW/JJBLC/4XCJAgwDJJIyCZQpNBGQOPSZAyFF4ZlCGQIwMGQRMDMoMKJAIwMGQYZEC4IwOGQYaCAYQwODIwXTDQgWLA="));

let imageIcons =
atob("DHjBAP//wxgAABgDwH4L4fwuh/D6D8B4AwACAHAHACAYAYQ848498Z8A4AQAAEAkQUgAAAAJAZjw/gdgYAAAABgAACACEIQIgmBwFzb4AAAAAHAPgPgPgPgPgIgIgIgNgHAgRw5w75/w/w8AAGAPAPAfgPAEAOAOAOAfAfA/g/h/w/g/gOAGAfg/x55w7gf/9/5/4/wfgGAEAOAXA7hdzu53Q7gdAOAEAAAOAOAOAOD/7/7/4OAOAOAOAAA=");
E.toArrayBuffer(imageIcons)[1] = 12;

const personalEffectPos = [
  37, 89,
  49, 66,
  66, 49,
  86, 39,
  108, 33
];

const envEffectPos = [
  200, 89,
  187, 65,
  166, 47,
  140, 37
];

const effectColors = [ "#c00", "#ff6a00",  "#347CA0", "#efb813", "#d782ff", "#00c721"]

function drawPersonalEffect(slotI, effectI) {
  let x = personalEffectPos[slotI <<= 1], y = personalEffectPos[slotI + 1];
  g.setColor(effectColors[effectI]).setBgColor("#111")
   .fillPolyAA([ x - 10, y + 11, x, y - 9, x + 10, y + 11 ], true)
   .setPixel(x - 9, y + 11, effectColors[effectI]); // fixing poly glitch
  if (effectI == 3) x--;
  if (effectI == 2 || effectI == 3) y++;
  g.drawImage(imageIcons, x - 6, y - 2, { frame: effectI });
}

function drawEnvEffect(slotI, effectI) {
  let x = envEffectPos[slotI <<= 1], y = envEffectPos[slotI + 1];
  g.setColor(effectColors[effectI]).setBgColor("#111")
   .fillCircleAA(x, y, 9);
  if ((effectI & 1) == 0) x--;
  g.drawImage(imageIcons, x - 5, y - 5, { frame: 5 + effectI });
}

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

const gfx_name = Graphics.createArrayBuffer(7, 12 * 12, 2, { msb: true });
const img_name = { width: 7, height: 12, bpp: 2, buffer: gfx_name.buffer, transparent: 0 };
gfx_name.transparent = 0;
let lastName = ""; // TODO: reuse same characters, skip space and so on...

function prepareName(name) {
  gfx_name.clear(true);
  gfx_name.setColor("#fff");
  gfx_name.setFontArchitekt12();
  for (var i = 0; i < name.length; i++)
    gfx_name.drawString(name[i], 0, i * 12, false);
}

function drawCircledChar(ch, angle) {
  const r = 57;
  const x = 120 + Math.cos(angle) * r;
  const y = 120 + Math.sin(angle) * r;
  g.drawImage(img_name, x, y, { frame: ch, rotate: -3.1415926 / 2 + angle, filter: true });
}

const anglePerChar = 0.17;
function drawName(name, angle) {
  if (lastName != name)
    prepareName(lastName = name);
  var curAngle = angle + name.length * anglePerChar * 0.5;
  for (var i = 0; i < name.length; i++, curAngle -= anglePerChar)
    drawCircledChar(i, curAngle);
}

function fullDraw(o2, co2) {
  g.setBgColor("#111");
  g.clear(false);
  g.drawBackground();
  g.drawTicks(0.3);
  drawO2CO2(o2, co2);
  g.setFontArchitekt12(1);
  g.setColor("#fff");
  g.drawString("O2", 120 - 92, 104);
  g.drawString("CO2", 120 + 73, 104);
  
  drawMoon(o2 / 63);
  g.drawImage(imagePlanet, 70, 70);
  
  g.setColor("#fff");
  drawName('HEMERLO IV', o2 / 63 * 3.141592653);
  
  drawPersonalEffect(0, 4);
  drawPersonalEffect(1, 1);
  drawPersonalEffect(2, 2);
  drawPersonalEffect(3, 3);
  drawPersonalEffect(4, 0);
  drawEnvEffect(0, 0);
  drawEnvEffect(1, 1);
  drawEnvEffect(2, 2);
  drawEnvEffect(3, 3);
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