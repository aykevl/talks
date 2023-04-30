package main

import (
	"image/color"
	"machine"
	"time"

	"github.com/aykevl/ledsgo"
	"tinygo.org/x/drivers/ws2812"
)

func main() {
	// Configure the WS2812 pin.
	machine.WS2812.Configure(machine.PinConfig{Mode: machine.PinOutput})

	// Create a driver object.
	ws := ws2812.New(machine.WS2812)

	// Create an array of LEDs.
	leds := make([]color.RGBA, 10)

	// Endless loop.
	for {
		// Update the 'leds' slice to the new color.
		now := time.Now()
		for i := range leds {
			c := ledsgo.Color{
				H: uint16(now.UnixNano()>>16) + uint16(i*2000),
				S: 255,
				V: 255,
			}
			leds[i] = c.Spectrum()
		}

		// Write the new colors to the LEDs.
		ws.WriteColors(leds)

		// Sleep for a bit.
		time.Sleep(time.Second / 30)
	}
}
