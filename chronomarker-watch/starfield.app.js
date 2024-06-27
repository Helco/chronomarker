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
g.reset();
g.setColor("#fff");
g.clear(false);
g.drawBackground();
g.drawTicks(0.3);

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
  
  g.clear(false);
  g.drawBackground();
  g.drawTicks(0.3);
  drawO2CO2(o2, co2);
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
