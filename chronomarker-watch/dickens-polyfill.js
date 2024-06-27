if (!global.settings) {
    global.settings = { "ble": true, "log": false, "echo": false, "logBLE": false,
        "timeout": 20, "autoDim": 0, "timeoutOnCharge": 0, "vibrate": true,
        "beep": "vib",
        "timezone": 2, "HID": false, "clock": null, "12hour": true, "dateFormat": 0,
        "brightness": 0.5, "ANCS": false, "AMS": false, "CTS": false, "stepGoal": 4000,
        "fahrenheit": false,
        "options": { "wakeOnFaceUp": false, "wakeOnTwist": true, "wakeOnTouch": true, "twistThreshold": 600,
          "twistMaxX": 4800, "twistMaxY": 1600, "twistTimeout": 1000 },
        "logSerial": false, "quiet": 0 };
}

if (!g.fillSeg)
    g.fillSeg = function (a, ar, r1, r2) {
        var a1 = a - ar,
            a2 = a + ar,
            sa1 = Math.sin(a1),
            ca1 = Math.cos(a1),
            sa2 = Math.sin(a2),
            ca2 = Math.cos(a2)
        return this.fillPolyAA([
            119 + r1 * sa1,
            119 - r1 * ca1,
            119 + r1 * sa2,
            119 - r1 * ca2,
            119 + r2 * sa2,
            119 - r2 * ca2,
            119 + r2 * sa1,
            119 - r2 * ca1
        ])
    }

if (!g.drawSeg)
    g.drawSeg = function (a, ar, r) {
        var a1 = a - ar,
            a2 = a + ar,
            s = Math.sin,
            c = Math.cos
        return this.drawLineAA(
            119 + r * s(a1),
            119 - r * c(a1),
            119 + r * s(a2),
            119 - r * c(a2)
        )
    };

if (!g.drawSlice)
    g.drawSlice = function (a1, a2, r1, r2) {
        if (a2 < a1) return;
        if (a2 - a1 > 3.8) a2 = a1 + 3.8
        var a,
            res = 8
        var poly = []
        for (var i = a1 * res; i < a2 * res; i++) {
            a = i / res
            poly.push(119 + r2 * Math.sin(a), 119 - r2 * Math.cos(a))
            poly.unshift(119 + r1 * Math.sin(a), 119 - r1 * Math.cos(a))
        }
        a = a2
        poly.push(119 + r2 * Math.sin(a), 119 - r2 * Math.cos(a))
        poly.unshift(119 + r1 * Math.sin(a), 119 - r1 * Math.cos(a))
        return this.fillPolyAA(poly)
    };

if (!g.fillArc)
    g.fillArc = function (a1, a2, r) {
        if (a2 < a1) return
        if (a2 - a1 > 6.28) a2 = a1 + 6.28
        var a,
            res = 8
        var poly = []
        for (var i = a1 * res; i < a2 * res; i++) {
            a = i / res
            poly.push(119 + r * Math.sin(a), 119 - r * Math.cos(a))
        }
        a = a2
        poly.push(119 + r * Math.sin(a), 119 - r * Math.cos(a))
        return this.fillPolyAA(poly)
    };

if (!g.fillCircleAA)
    g.fillCircleAA = function (x, y, r) {
        var p = []
        for (var a = 6.28319; a >= 0; a -= 0.1)
            p.push(x + r * Math.sin(a), y + r * Math.cos(a))
        return this.fillPolyAA(p)
    };

if (!g.fillPointer)
    g.fillPointer = function (a, ar, r1, r2) {
        var a1 = a - ar,
            a2 = a + ar,
            s = Math.sin,
            c = Math.cos
        return this.fillPolyAA([
            119 + r1 * s(a1),
            119 - r1 * c(a1),
            119 + r2 * s(a),
            119 - r2 * c(a),
            119 + r1 * s(a2),
            119 - r1 * c(a2)
        ])
    };

if (!g.drawCentredText)
    g.drawCentredText = function (text) {
        lines = text.split('\n')
        this.setFontGrotesk20().setColor(-1).setBgColor(0).setFontAlign(0, 0)
        lines.forEach((s, i) => {
            this.drawString(s, 119, 119 - 12 * (lines.length - 1) + i * 24)
        })
    };

if (!g.fillAnnulus)
    g.fillAnnulus = function (x, y, r1, r2) {
        this.fillCircle(x, y, r2).setColor(0).fillCircle(x, y, r1);
    };

if (!g.setFontArchitekt15)
    g.setFontArchitekt15 = function () {
        return this.setFont("6x8", 1);
    };

var Dickens = {
    debug: { timing: false, power: false },
    clock: {
        secondHand: true,
        secondTimer: 0,
        minuteTimer: 0,
        northAngle: 0,
        showNorth: false
    },
    LCD: {
        rotation: { current: 0, debounce: 0, timer: null },
        dimTimer: null
    },
    icon9: {
        up: atob('CQmBAAAAAgKCIgoCAAAA'),
        down: atob('CQmBAAAAICgiIKAgAAAA'),
        left: atob('CQmBAAICAgICAIAgCAIA'),
        right: atob('CQmBACAIAgCAICAgICAA'),
        select: atob('CQmBAAAOCIgkEgiIOAAA'),
        back: atob('CQmBACAwP4wiCAQCAn4A'),
        tick: atob('CQmBAAAAQEBEQUBAAAAA'),
        cross: atob('CQmBAAAgiIKAgKCIggAA'),
        circle: atob('CQmBAD4goDAYDAYCgj4A'),
        menu: atob('CQmBAAB/wAAP+AAB/wAA'),
        clock: atob('CQmBAD4gojEY7AYCgj4A'),
        message: atob('CQmBAH9Ab7Ab7AXMFAYA'),
        chart: atob('CQmBAAIBCIVCrVarVf+A'),
        music: atob('CQmBAD+QSCQSCw+PhgAA'),
        moon: atob('CQmBABwHA8Hg8Hg8HBwA'),
        sun: atob('CQmBAIighwRKKRBwgoiA'),
        play: atob('CQmBACAYDgeD4eDgYCAA'),
        pause: atob('CQmBAGMxmMxmMxmMxmMA'),
        torch: atob('CQmBAD4AD4fBwOBwOBwA'),
        sine: atob('CQmBACAoFBEYiCgUBAAA'),
        cloud: atob('CQmBAAAGB4Pn///+/gAA'),
        timer: atob('CQmBAAAOEIQpFAoEhDwA'),
        stopwatch: atob('CQmBABwAB0REkkkERBwA')
    },
    buttonIcons: [],
    setBLEServices: function() {
        console.log("POLYFILL: Set BLE services")
    }
};
global.Dickens = Dickens;

g.drawTicks = function (b) {
    b = (b || 0) - 0.105
    var s = Math.sin,
        c = Math.cos,
        sa,
        ca,
        sb,
        cb,
        cl = this.setColor.bind(this)
    this.l = this.drawLineAA
    for (var i = 0; i <= 15; i++) {
        sb = (sa = s((b += 0.105)) * 64) * 1.06
        cb = (ca = c(b) * 64) * 1.06
        cl(i % 5 ? '#444' : '#bbb')
            .l(119 + sa, 119 + ca, 119 + sb, 119 + cb)
            .l(119 - ca, 119 + sa, 119 - cb, 119 + sb)
            .l(119 + ca, 119 - sa, 119 + cb, 119 - sb)
            .l(119 - sa, 119 - ca, 119 - sb, 119 - cb)
    }
    delete g.l
}

g.drawBackground = function () {
    this.reset()
        ;['hr', 'min', 'sec', 'north'].forEach(n => {
            delete Dickens.clock[n]
        })
    this.setColor(0).fillAnnulus(119, 119, 98, 117)
    this.setColor('#222').fillAnnulus(119, 119, 117, 122)
    this.drawCircleAA(119, 119, 116)
    this.setColor('#555').drawCircleAA(119, 119, 99)
}

g.drawHourMarkings = function () {
    this.setColor('#888')
        .setFontArchitekt15()
        .setFontAlign(0, 0)
        .drawString(12, 119, 11)
        .drawString(3, 226, 119)
        .drawString(6, 119, 226)
        .drawString(9, 11, 119)
    if (Dickens.LCD.rotation.current == 0) {
        var b = Dickens.buttonIcons
        var i = Dickens.icon9
        if (b[0] in i) g.drawImage(i[b[0]], 211, 66)
        if (b[1] in i) g.drawImage(i[b[1]], 211, 163)
        if (b[2] in i) g.drawImage(i[b[2]], 18, 163)
        if (b[3] in i) g.drawImage(i[b[3]], 18, 66)
    }
}

Dickens.drawSemicircle = function (options) {
    if (Dickens.sweepT) clearInterval(Dickens.sweepT)
    delete Dickens.sweepT
    if (!options) return;
    var a1 = 4.7124
    var a2 = a1 - options.amt * 3.1416
    if (Dickens.sweepA == undefined) Dickens.sweepA = a1
    g.reset()
    var fg = -1,
        bg = '#555'
    if (options.inverted) {
        a1 = 1.5708
        t = fg
        fg = bg
        bg = t
    }
    if (options.clear) {
        Dickens.sweepA = a1
        g.setColor('#ccc')
        //  .drawImage(options.iconLeft, 30, 96)
        //  .drawImage(icons.chequeredFlag, 193, 106)
        g.setColor('#555').drawSlice(1.5708, 4.7124, 72, 95)
        // Dickens.pauseSeconds()
    }
    var step = a2 < Dickens.sweepA ? 0.005 : -0.005
    var ang = Dickens.sweepA + step
    Dickens.sweepT = setInterval(_ => {
        g.setBgColor(0)
        var n = step > 0 ? 1 : 0
        g.setColor(n ? fg : bg).drawSlice(
            Math.min(ang, Dickens.sweepA),
            Math.max(ang, Dickens.sweepA),
            72 + n,
            95 - n
        )
        g.flip()
        Dickens.sweepA = ang
        if (Math.abs(ang - a2) > 0.3) {
            if (Math.abs(step) < 0.07) step += Math.sign(step) * 0.005
        } else if (Math.abs(step) > 0.02) step -= Math.sign(step) * 0.01
        ang -= step
        if ((ang - a2) * Math.sign(step) < 0) {
            if (Dickens.sweepT) clearInterval(Dickens.sweepT)
            delete Dickens.sweepT
            if (options.clear) Dickens.showSeconds(true)
        }
    }, 18)
}

const architekt15 = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACqqSgD//TwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD/AAABpAAAAAAAAD/AAABpAAAAAAAAAAAAAABAQAAXl5QB///gADg4AADg5QB///gADg4AAAAAAAAAAAAFAEAB/wPgDx4DwHgsB0vgOA+DwLBwC4D7wAtA/QAAAAAAZABQC/gLwDiw9AB/XwAAA+aAAH1/gAuC2wD0A/gAAAAAAAAAAA/R/QC2/zwDg/BwC733wA/A/AAAA/wAAAxwAAAAAAAAAAAAAAAAAAAAD5AAAD/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAB///QD1VXwDQABwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADQABwD1VXwB///QAAAAAAAAAAAAAAAAAAAAAAAAADzwAAB/QAAA/AAAD7wAACRgAAAAAAAAAAAAAAAAAAANAAAANAAAKupAAP/9AAANAAAANAAAANAAAAAAAAAAAAAAAAAAAAAAAAAAOAAAD8AAADwAAAAAAAAAAAAAAAAAEAAAANAAAANAAAANAAAANAAAANAAAANAAAANAAAAAAAAAAAAAAAAAAAAAAAAADwAAADwAAAAAAAAAAAAAAAAAAAAAAABQAAAPwAAB8AAALwAAA+AAAD0AABvQAAD8AAAAAAAAAaqpAB///QDwC7wDgLiwDQuBwDi4CwD7qrwB///QAAAAAAAAAADQAAwDQAAwDqqqwD///wAAAAwAAAAwAAAAwAAAAAAEAKgB9C/wDwHgwDgPAwDgdAwDw8AwB/0AwAfQAwAAAAAAEAFAB8APgDwECwDgNAwDgNAwDgNBwD5/rwA/7/QAAAAAAAGkAAC/0AAvg0AD4A0AAAG6gAAP/wAAA0AAAAkAAAAAABqkFAD/9PgDQNCwDQNAwDQNAwDQNBwDQPrwDQH/QAAAAAAABpAAAf/gAH+CwB/dAwD0NAwAANBwAAPrwAAH/QAAAAAAAAAADQAAADQABQDQAfwDQD8ADQvQADr8AAD/QAAAAAAAAGRpAB/7/gDwuCwDgNAwDgNAwDgNBwD5/rwA/7/QAAAAAAGQAAB/8AADwdAADgNAQDgNLwDgP+AD6/gAA/4AAAAAAAAAAAAAAAAAAAAAAAA8DwAA8DwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOAA8D8AA8HwAAAAAAAAAAAAAAAAAFAAAAfgAAA90AADw8AAHQPAAPAHgA8ACwAoAAwAAAAAAAAAAAPA8AAPA8AAPA8AAPA8AAPA8AAPA8AAKAoAAAAAAAAAAQA8ABwAtADwAPALQADweAAB08AAAvwAAAPQAAAAAAAEAAAB9AAADwAAADgBCgDgPTwDw8AAC/0AAAvQAAAAAAAAEApAB8L/wDwOCwDgOqwDgP/wDgABwD6qrwA///QAAAAAAAGqgAH//wB/g0AD0A0ADgA0AD0A0AB/q6gAH//wAAAAABqqqgD///wDQNAwDQNAwDQNAwDgNBwD6/rwA/7/QAAAAAAGqpAB///gDwADwDgABwDQAAwDgABwD4AHwA8APQAAAAABqqqgD///wDQAAwDQAAwDQAAwDgABwD6qrwB///QAAAAABqqqgD///wDQNAwDQNAwDQNAwDQNAwDQNAwDQNAwAAAAABqqqgD///wDQNAADQNAADQNAADQNAADQNAADQNAAAAAAAAGqpAB///QDwADwDgAAwDQEAwDgNBwD4OrwB8P/QAAAAABqqqgD///wAANAAAANAAAANAAAANAABquqgD///wAAAAAAAAAADQAAwDQAAwDqqqwD///wDQAAwDQAAwDQAAwAAAAAAAAFADQAPgDQACwDQAAwDQAAwDQABwDqqrwD///QAAAAABqqqgD///wAANAAAANAAAA/AAAD70ABvQ+QD8APwAAAAABqqqgD///wAAAAwAAAAwAAAAwAAAAwAAAAwAAAAwAAAAABqqqgD///wAvQAAAH9AAAB/AAALwAAB/qqgD///wAAAAABqqqgD///wAfQAAAH5AAABvQAAAD8ABqqvgD///wAAAAAAaqpAB///QDwADwDgACwDQABwDgACwD6qrwB///QAAAAABqqqgD///wDQNAADQNAADQNAADgNAAD68AABv0AAAAAAAAGqpAB///gDwACwDgAFwDQAvwDgABwD6qrwA///QAAAAABqqqgD///wDQNAADQNAADQPAADgP0AC68+QAvwLwAAAAAAFAEAB/wPgDx4DwDgsBwDgOAwDwLBwC4D7wAtA/AAAAAAAAAAADQAAADQAAADqqqgD///wDQAAADQAAADQAAAAAAAABqqpAD///gAAADwAAABwAAAAwAAABwBqqrwD///QAAAAABqpAAD//0AAAB/gAAAHwAAABwAAAfwBqr+AD//QAAAAAABqqqQD///wAAA/AAAL0AAAvgAAAC9ABqqvgD///wAAAAABkABQD+AfwALx8AAC/wAAA/AAAD70ABvQ+QD8AbwAAAAABkAAAD+AAAALwAAAC+qgAA//wAD0AABvQAAD8AAAAAAAAAAAAQDQAHwDQA/wDQL0wDR/AwDb0AwD+AAwDwAAwAAAAAAAAAAAAAAAAVVVAD///wDQABwDQAAwAAAAAAAAAAAAAAABQAAAD9AAAAPgAAAC8AAAAfAAAAD0AAAA+QAAALwAAAAAAAAAAAAAAAAAAAADQAAwDVVVwD///wAAAAAAAAAAAAAAA==";
g.setFontArchitekt15 = function(scale) {
    this.setFontCustom(atob(architekt15), 32, 9, 15 + (scale << 8) + (2 << 16)*209);
    return this;
};

if (typeof global.NRF !== 'object')
    global.NRF = {};
if (!NRF.disconnect)
    NRF.disconnect = function() {};
if (!NRF.setServices)
    NRF.setServices = function(a, b) {};
if (!NRF.setAdvertising)
    NRF.setAdvertising = function(a, b) {};
if (!NRF.updateServices)
    NRF.updateServices = function(a) {};
