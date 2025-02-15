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
    g.drawSlice = function (a1, a2, r1, r2, ox, oy) {
        if (ox === undefined)
            ox = 119;
        if (oy === undefined)
            oy = 119;
        if (a2 < a1) return;
        if (a2 - a1 > 3.8) a2 = a1 + 3.8
        var a,
            res = 8
        var poly = []
        for (var i = a1 * res; i < a2 * res; i++) {
            a = i / res
            poly.push(ox + r2 * Math.sin(a), oy - r2 * Math.cos(a))
            poly.unshift(ox + r1 * Math.sin(a), oy - r1 * Math.cos(a))
        }
        a = a2
        poly.push(ox + r2 * Math.sin(a), oy - r2 * Math.cos(a))
        poly.unshift(ox + r1 * Math.sin(a), oy - r1 * Math.cos(a))
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
        this.drawSlice(0, 3.141592653, r1, r2, x, y);
        this.drawSlice(3.141592653, 2*3.141592653, r1, r2, x, y);
        return this;
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

const architekt12  = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP/zwAAAAAAAAAAAAAAAAAAAAPwAAAAAAPwAAAAAAAAAAAAAAAwMAP//wAwMAP//wAwMAAAAAAAAAH9DQNDBw8DA8NDRwHx/QAAAAAAAAD8HwD88AADUAAdPwDwPwAAAAAAFAH9vQNHwwLf0wC0fQAA2wAAAAAAAAAAAAAAAAP0AAAAAAAAAAAAAAAAAAAAAAB//0HQAdNAAHAAAAAAAAAAAAAAAANAAHHQAdB//0AAAAAAAAAAAAJGAAHdAAK2AAHdAAJGAAAAAAAAAAAAwAAAwAA//wAAwAAAwAAAAAAAAAAAAAAAAGAAD4AAAAAAAAAAAAAAAAAAwAAAwAAAwAAAwAAAwAAAAAAAAAAAAAAAAAAADwAAAAAAAAAAAAAAAAAAHwAAdAAH0AAdAAH0AAAAAAAAAAH//QMB0wMHQwMdAwH//QAAAAAAAAMAAwMAAwP//wAAAwAAAwAAAAAAAAHQfwMBwwMDQwMHAwH9AwAAAAAAAAHADQMDAwMDAwMDAwH9/QAAAAAAAAP/8AAAMAAAMAAD/wAAMAAAAAAAAAP/DQMDAwMDAwMDAwMB/QAAAAAAAAAL/QC/AwPTAwADAwAB/QAAAAAAAAMAAAMADwMA9AMPQAP0AAAAAAAAAAH9/QMDAwMDAwMDAwH9/QAAAAAAAAH9AAMDAAMDHwMD+AH/gAAAAAAAAAAAAAAAAAA8DwAAAAAAAAAAAAAAAAAAAAAAAGA8D4AAAAAAAAAAAAACgAAH0AAddAA0HABwDQDQBwAAAAAAAAAwMAAwMAAwMAAwMAAwMAAAAAAAAADQBwBwDQA0HAAdcAAH0AACgAAAAAHwAAMAAAMBzwMHAAH9AAAAAAAAAAHR/QMDAwMC/wMAAwH//QAAAAAAAAB//wHQMANAMAHQMAB//wAAAAAAAAP//wMDAwMDAwMDAwH9/QAAAAAAAAH//QMAAwMAAwMAAwHADQAAAAAAAAP//wMAAwMAAwNABwH//QAAAAAAAAP//wMDAwMDAwMDAwMDAwAAAAAAAAP//wMDAAMDAAMDAAMDAAAAAAAAAAH//QMAAwMBAwMDAwHD/wAAAAAAAAP//wADAAADAAADAAP//wAAAAAAAAMAAwMAAwP//wMAAwMAAwAAAAAAAAMADQMAAwMAAwMAAwP//wAAAAAAAAP//wADAAAHQAAd0AP0fwAAAAAAAAP//wAAAwAAAwAAAwAAAwAAAAAAAAP//wB0AAAfQAB0AAP//wAAAAAAAAP//wB9AAAHQAAB9AP//wAAAAAAAAH//QMAAwMAAwMAAwH//QAAAAAAAAP//wMDAAMDAAMDAAH9AAAAAAAAAAH//QMAAwMAPwMAAwH//QAAAAAAAAP//wMDAAMDwAMDcAH9HwAAAAAAAAH9HQMHAwMDAwMBwwHQfQAAAAAAAAMAAAMAAAP//wMAAAMAAAAAAAAAAAP//QAAAwAAAwAAAwP//QAAAAAAAAP/9AAAHQAAAwAAHQP/9AAAAAAAAAP//wAAdAAD0AAAdAP//wAAAAAAAAP0fwAd0AAHQAAd0AP0fwAAAAAAAAP0AAAdAAAH/wAdAAP0AAAAAAAAAAMAHwMA9wMHQwN9AwPQAwAAAAAAAAAAAAP//8MAAMMAAMAAAAAAAAAAAAH0AAAdAAAH0AAAdAAAHwAAAAAAAAAAAAMAAMMAAMP//8AAAAAAAA";
Graphics.prototype.setFontArchitekt12 = function(scale) {
    this.setFontCustom(atob(architekt12), 32, 7, 12 + (scale << 8) + (2 << 16));
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

const C0 = '0'.charCodeAt(0);
const C9 = '9'.charCodeAt(0);
const CA = 'A'.charCodeAt(0);
const CF = 'F'.charCodeAt(0);
function hexChar(l, i)
{
    var ch = l.charCodeAt(i);
    ch = ch >= C0 && ch <= C9 ? ch - C0
        : ch >= CA && ch <= CF ? 10 + ch - CA
        : -1;
    if (ch < 0)
        throw new Error(`Cannot parse ${JSON.stringify(l.charAt(i))}`);
    process.memory(true);
    return ch;
}

function hexStringToBuffer(l)
{
    var buffer = new ArrayBuffer(l.length / 2);
    var view = new Uint8Array(buffer, 0, buffer.byteLength);
    for (let i = 0; i < buffer.byteLength; i++)
    {
        view[i] = (hexChar(l, i * 2) << 4) | hexChar(l, i * 2 + 1);
    }
    process.memory(true);
    return buffer;
}

const messages = `
0DD028A730CC9624100A
0201
8200
02688046398561B62401
2200
0612380026012040614100030000
2D6E89D372D996C0FCB2A10CC821998674FABA136643B7A4433907C6D725533A7D5DC1A31200
C207
E207
2D6E89D372D996C0FCB2A10CC821998674FABA136643B7A4433907C6D725533A7D5D
2B00
C207
8207
A207
C207
E207
C207
ED1E99CEEC3B867449026CDA8A404500
A207
8207
6207
4207
4206
2206
0206
E205
C205
C6DAF074433999363CDD503E794E01140200
E205
0206
2206
ED9C2987235DCB2509B0692B02250900
`.split("\n");

const msgDelay = 500;
let msgI = -((3000 / msgDelay)|0);
const msgInt = setInterval(() =>
{
    msgI++;
    if (msgI >= messages.length) {
        clearInterval(msgInt);
        return;
    }
    if (msgI < 0)
        return;
    process.memory(true);
    global.handlePacket(hexStringToBuffer(messages[msgI]));
    process.memory(true);
}, msgDelay);
