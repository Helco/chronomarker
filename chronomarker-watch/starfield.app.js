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
g.drawHourMarkings();
g.drawTicks(0.3);
//g.drawSlice(1.0, 2.0, 72, 95);

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
