if (typeof Dickens !== 'object') require('dickens-polyfill.js')
if (settings.log) log('Running starfield.app.js')

var notifyT = 0
var nextHeartbeatMessage = 0

function setStarfieldBLEServices () {
  if (notifyT != 0) clearInterval(notifyT)
  let svc = {
    ancs: false,
    ams: false,
    cts: false,
    uart: true,
    advertise: [0x7201]
  }
  NRF.disconnect()
  NRF.setServices(
    {
      0x7201: {
        0xc520: {
          writable: true,
          notify: true,
          maxLen: 128,
          value: [nextHeartbeatMessage],
          onWrite: function (evt) {
            handlePacket(evt.data);
          }
        }
      }
    },
    svc
  )
  NRF.setAdvertising(
    {},
    {
      showName: false,
      manufacturer: 0x7201,
      manufacturerData: [1]
    }
  )
  notifyT = setInterval(() => {
    try {
      NRF.updateServices({
        0x7201: {
          0xc520: {
            value: [nextHeartbeatMessage],
            notify: true
          }
        }
      })
    } catch (e) {}
  }, 3000)
  console.log('set new services')
}

setStarfieldBLEServices()
Bangle.setLCDTimeout(0)

const maxO2CO2 = 63
const o2StartAngle = 1.5 * 3.141592653
const co2StartAngle = 0.5 * 3.141592653
function drawO2CO2 (o2, co2, clearFirst) {
  if (clearFirst == true) {
    g.setColor('#111').drawSlice(co2StartAngle, o2StartAngle, 73, 96);
  }
  if (o2 > 0 && co2 > 0) o2--, co2--
  if (o2 > 0)
    g.setColor('#fff').drawSlice(
      o2StartAngle - (3.141592653 * o2) / maxO2CO2,
      o2StartAngle,
      74,
      95
    )
  if (co2 > 0)
    g.setColor('#f00').drawSlice(
      co2StartAngle,
      co2StartAngle + (3.141592653 * co2) / maxO2CO2,
      74,
      95
    )
}

const imagePlanet = require('heatshrink').decompress(
  atob(
    'slkwUAABMD/4AB4APKAA4WCAAX8C6E/CgIEBlYZRh//+AFChQfCJpwqB1QEBlQDCGR0PBwIVBAAOADgU/GRgNJhUDGRYMKMwQyKn/6IwIKGlRWDPRPqMAowC0DoCJJKqDgQ0DhQ4D35LIh4JFGoQ2El4nDMQrTNlBkHUBgAChCXHh6TC1AYLh5LGEAR5CDRMoIQ88AgaUEDA0ADAwGGlTJFAALKBRopRHYgQaEhQEBX4rFGJwotBAgIIBhYrEYpbjDDYIcBLon+BgQaJlRNDgX/bwf+BQJcFABWPDAZPDLALhLAAOvO4R7GLYgAI0YYD9RwFYwIZKkZGCn5ECJIIUDNBUgSwStFCgYeBDJEgngDBWQjWEAgKBIkAuCcggABhQuDlQZHkEPVorZEGYaAH0CrBgatFDIYUCMw+gCwLGGDIgUCKIJxEwBIBJgI/BLIyAEhQMDlR6Ch4OBcAoUDAwboCQYfwn7BFGgogEJoI5DDAIsEfQpMFAAv8fAr6FJgwYFfYR5FJgwYHnk8KQQmEFgoyIDAIjBS4RMJZgYYE/wIDSgjbGag08/SnJlRGEGQ0//CUGAgYsEGQ0P+AYFOgkKMooYMGRZREDBAyEFgplEDAa6FGQYsFAogYBcARGEGQh4FHAYYBAoUqB4jLEDAkCJYQYBYJCTDCQYjFh/yBQmqCgZLMh75FhQQDAYaXFHgQYGCAgDDcYqoCDA5HDMAawFEYUP/DgGI4YUDDAqdBDBBHDDgcoWA0P/gdB1SsHMghZGn4YBlT5ELgYcDDApRBnk8KoSKEI4R9ESwsoDAIJDJYZcDTQxRDnk+BIYUDMAYYJgU8n4JDLg4gCZA0C/gYEPI5OGDBJ5HJwwAD+EPDAhDDDB/CDAZgCNYgYL4D3EAYQYMgfAgfwL4QYVLAYYJVwxIBgH4Yg4YGcAsPAQP+HgY1CPgwYGngCBn6rDVxIYFhX8DAWAJYJYDCIYfCLAYAB1fwJoXggWq1RmGDAYwE0YYD/AHBWI5UFKQStBWIX6dQoRDKIQYE1CtCAAPyQ4o1DDATGDNwKtCAAP8CQbYEP4SUETgKqBAAU/lQMDRoYVCPYgpBVoR9C5QyCgR/GZIQfCgaUCPoXwVwIABLYZ4CD4YcB157DMgUAC4p4CNIYGBgW/VAppEIQhpGlRiEDAXgA4pfCYoYGB1YYGh7iGL4JJE1ECx57EPoX+ZIi1CXAZNBhU/PYpLC9QpChQwCSYpJHDAXAWAYzEJoUC15JGJYQiCchCTBJJAAB//qVoeqGAZ6B1f/C5BLB/z8HlWglW/JJBLC/4XCJAgwDJJIyCZQpNBGQOPSZAyFF4ZlCGQIwMGQRMDMoMKJAIwMGQYZEC4IwOGQYaCAYQwODIwXTDQgWLA='
  )
)

let imageIcons = atob( 'DHjBAP//wxgAABgDwH4L4fwuh/D6D8B4AwACAHAHACAYAYQ848498Z8A4AQAAEAkQUgAAAAJAZjw/gdgYAAAABgAACACEIQIgmBwFzb4AAAAAHAPgPgPgPgPgIgIgIgNgHAgRw5w75/w/w8AAGAPAPAfgPAEAOAOAOAfAfA/g/h/w/g/gOAGAfg/x55w7gf/9/5/4/wfgGAEAOAXA7hdzu53Q7gdAOAEAAAOAOAOAOD/7/7/4OAOAOAOAAA='
)
E.toArrayBuffer(imageIcons)[1] = 12

const personalEffectPos = [37, 89, 49, 66, 66, 49, 86, 39, 108, 33]

const envEffectPos = [200, 89, 187, 65, 166, 47, 140, 37]

const effectColors = [
  '#c00',
  '#ff6a00',
  '#347CA0',
  '#efb813',
  '#d782ff',
  '#00c721'
]

function drawPersonalEffect (slotI, effectI) {
  let x = personalEffectPos[(slotI <<= 1)],
    y = personalEffectPos[slotI + 1]
  g.setColor(effectColors[effectI])
    .setBgColor('#111')
    .fillPolyAA([x - 10, y + 11, x, y - 9, x + 10, y + 11], true)
    .setPixel(x - 9, y + 11, effectColors[effectI]) // fixing poly glitch
  if (effectI == 3) x--
  if (effectI == 2 || effectI == 3) y++
  g.drawImage(imageIcons, x - 6, y - 2, { frame: effectI })
}

function drawEnvEffect (slotI, effectI) {
  let x = envEffectPos[(slotI <<= 1)],
    y = envEffectPos[slotI + 1]
  g.setColor(effectColors[effectI]).setBgColor('#111').fillCircleAA(x, y, 9)
  if ((effectI & 1) == 0) x--
  g.drawImage(imageIcons, x - 5, y - 5, { frame: 5 + effectI })
}

function drawMoon (phase) {
  var x = 119,
    y = 119,
    r0 = 50,
    tilt = 1
  var r1, r2
  if (phase < 0.5) {
    r1 = r0
    r2 = -r0 + phase * 4 * r0
  } else {
    r1 = r0
    r2 = r0 - (phase - 0.5) * 4 * r0
    tilt += 3.141592653
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
    g.setColor('#ddd').fillPolyAA(poly)
  }
}

const gfx_name = Graphics.createArrayBuffer(7, 12 * 12, 2, { msb: true })
const img_name = {
  width: 7,
  height: 12,
  bpp: 2,
  buffer: gfx_name.buffer,
  transparent: 0
}
gfx_name.transparent = 0
let lastName = '' // TODO: reuse same characters, skip space and so on...

function prepareName (name) {
  gfx_name.clear(true)
  gfx_name.setColor('#fff')
  gfx_name.setFontArchitekt12()
  for (var i = 0; i < name.length; i++)
    gfx_name.drawString(name[i], 0, i * 12, false)
}

function drawCircledChar (ch, angle) {
  const r = 57
  const x = 120 + Math.cos(angle) * r
  const y = 120 + Math.sin(angle) * r
  g.drawImage(img_name, x, y, {
    frame: ch,
    rotate: -3.1415926 / 2 + angle,
    filter: true
  })
}

const anglePerChar = 0.17
function drawName (name, angle) {
  if (lastName !== name) prepareName((lastName = name))
  var curAngle = angle + name.length * anglePerChar * 0.5
  for (var i = 0; i < name.length; i++, curAngle -= anglePerChar)
    drawCircledChar(i, curAngle)
}

const game = {
  o2: maxO2CO2,
  co2: 0,
  heading: 0,
  time: 0,
  bodyName: '',
  locationName: '',
  personalEffects: [-1, -1, -1, -1, -1],
  envEffects: [-1, -1, -1, -1],
  planetGrav: 1,
  planetTemp: 20,
  planetOxygen: 100
};
function fullDraw () {
  g.setBgColor('#111')
  g.clear(false)
  g.drawBackground()
  g.drawTicks(0.3)
  drawO2CO2(game.o2, game.co2)
  g.setFontArchitekt12(1)
  g.setColor('#fff')
  g.drawString('O2', 120 - 92, 104)
  g.drawString('CO2', 120 + 73, 104)

  drawMoon(game.time / 63)
  g.drawImage(imagePlanet, 70, 70)

  g.setColor('#fff')
  drawName(game.bodyName, 0)

  for (let i = 0; i < 5 && game.personalEffects[i] >= 0; i++)
    drawPersonalEffect(i, game.personalEffects[i]);
  for (let i = 0; i < 4 && game.envEffects[i] >= 0; i++)
    drawEnvEffect(i, game.envEffects[i]);
}

class BitStream {
  /**
   * @param {ArrayBuffer} buffer
   */
  constructor (buffer) { this.reset(buffer); }
  reset (buffer) {
    this.buffer = buffer
    this.values = new Uint16Array(
      buffer,
      0,
      buffer.byteLength / 2
    )
    this.nextValue = 0
    this.bits = 0
    this.bitCount = 0
  }

  /**
   * @param {number} bitCount
   * @returns bits
   */
  read (bitCount) {
    if (bitCount < 0 || bitCount > 16) throw new Error('Invalid bit count')
    if (this.bitCount < bitCount) {
      if (this.nextValue >= this.values.length) throw new Error('end of stream')
      this.bits |= this.values[this.nextValue++] << this.bitCount
      this.bitCount += 16
    }
    this.bitCount -= bitCount
    const value = this.bits & ((1 << bitCount) - 1)
    this.bits >>= bitCount
    return value
  }

  readf (bitCount) {
    const i = this.read(bitCount)
    return i // / ((1 << bitCount) - 1);
  }

  reads (lengthBitCount) {
    const l = this.read(lengthBitCount)
    let s = ''
    for (let i = 0; i < l; i++) s += String.fromCharCode(32 + this.read(6))
    return s
  }

  bitsRemaining () {
    return (this.values.length - this.nextValue) * 16 + this.bitCount
  }

  hasAtLeast (n) {
    return this.bitsRemaining() >= n
  }
}

const CBit_O2CO2 = 0x01;
const CBit_Heading = 0x02;
const CBit_Time = 0x04;
const CBit_Names = 0x08;
const CBit_PlanetStats = 0x10;
const CBit_PersonalEffects = 0x20;
const CBit_EnvEffects = 0x40;
const CBit_PlayerFlags = 0x80;
function readEffects(arr, max, stream)
{
  let c = stream.read(3); if (c > max) c = max;
  let i = 0, w = 0;
  for (; i < c; i++)
  {
    let e = stream.read(3);
    if (e >= max)
      e = -1;
    else for (let j = 0; j < w; j++)
      if (arr[j] == e) {
        e = -1;
        break;
      }
    arr[w++] = e;
  }
  if (w < max)
    arr[w] = -1;
}
var stream = null;
function readPacket (packet) {
  let changeBits = 0;
 // try {
    if (stream == null) stream = new BitStream(packet);
    else stream.reset(packet);
    while (stream.hasAtLeast(5)) {
      let type = stream.read(5)
      switch (type) {
        case 0:
          return changeBits;
        case 1: // reset
          game.personalEffects.fill(-1);
          game.envEffects.fill(-1);
          break
        case 2: game.o2 = stream.readf(6); changeBits |= CBit_O2CO2; break
        case 3: game.co2 = stream.readf(6); changeBits |= CBit_O2CO2; break
        case 4: game.heading = stream.readf(7); changeBits |= CBit_Heading; break
        case 5: game.flags = stream.read(3); changeBits |= CBit_PlayerFlags; break
        case 8: game.time = stream.readf(6); changeBits |= CBit_Time; break
        case 6: game.bodyName = stream.reads(4); changeBits |= CBit_Names; break
        case 9: game.locationName = stream.reads(5); changeBits |= CBit_Names; break
        case 7:
          game.planetGrav = stream.readf(8);
          game.planetTemp = stream.read(11) - 294;
          game.planetOxygen = stream.read(7);
          changeBits |= CBit_PlanetStats;
          break
        case 10: readEffects(game.personalEffects, 5, stream); changeBits |= CBit_PersonalEffects; break
        case 11: readEffects(game.envEffects, 4, stream); changeBits |= CBit_EnvEffects; break;
        case 12:
        case 13:
          console.log(
            `  - ${['Positive', 'Negative'][type - 12]}: ${
              [
                'None',
                'Radiation',
                'Thermal',
                'Airborne',
                'Corrosive',
                'Cardio',
                'Skeletal',
                'Nervous',
                'Digestive',
                'Misc',
                'Restore'
              ][stream.read(4)]
            } "${stream.reads(5)}" "${stream.reads(5)}"`
          )
          break
        default: // we cannot trust any more data in this packet
          console.log(`  - Invalid packet type (${type})`)
          return changeBits;
      }
    }
  //} catch (e) {
  //  console.log('  - Invalid packet: ' + e)
  //}
  return changeBits;
}

let isFirst = true;

function handlePacket(packet) {
  Bangle.setLCDTimeout(0)
  const changeBits = readPacket(packet);
  if (isFirst) {
  fullDraw(); isFirst = false;
  }
  else
  {
    g.setBgColor('#111')
  drawO2CO2(game.o2, game.co2, true)
  }
  //console.log(process.memory());
}

function exitTo (nextApp) {
  if (notifyT != 0) clearInterval(notifyT)
  NRF.disconnect()
  Dickens.setBLEServices()
  Bangle.setLCDTimeout(10) // just paranoia, clock should set it to the default
  load(nextApp)
}


setWatch(_ => exitTo('clock.app.js'), BTN1, { edge: 1, repeat: true })
setWatch(_ => exitTo('clock.app.js'), BTN2, { edge: 1 })
setWatch(_ => exitTo('clock.app.js'), BTN4, { edge: 1 })
setWatch(_ => setStarfieldBLEServices(), BTN3, { edge: 1 })
