//if (settings.log) log('Running starfield.app.js')

g.reset();
g.setColor("#fff");
g.clear(false);

g.drawBackground();
g.drawHourMarkings();
g.drawTicks(0.3);

function exitTo (nextApp) {
    load(nextApp);
}

setWatch(_ => exitTo('clock.app.js'), BTN1, { edge: 1, repeat: true });
setWatch(_ => exitTo('clock.app.js'), BTN2, { edge: 1 });
setWatch(_ => exitTo('clock.app.js'), BTN3, { edge: 1 });
setWatch(_ => exitTo('clock.app.js'), BTN4, { edge: 1 });
