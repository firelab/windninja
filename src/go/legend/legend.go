package legend

import (
	"fmt"
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"io"

	"golang.org/x/image/font"
	"golang.org/x/image/font/inconsolata"
	"golang.org/x/image/math/fixed"
)

const (
	legendHeight = 180
	legendWidth  = 140
	numColors    = 5
)

var colorSchemes = []struct {
	name   string
	colors [5]color.Color
}{
	{
		"default",
		[5]color.Color{
			color.RGBA{0x00, 0x00, 0xff, 0xff},
			color.RGBA{0x00, 0xff, 0x00, 0xff},
			color.RGBA{0xff, 0xff, 0x00, 0xff},
			color.RGBA{0xff, 0x7f, 0x00, 0xff},
			color.RGBA{0xff, 0x00, 0x00, 0xff},
		},
	},
	{
		"oranges",
		[5]color.Color{
			color.RGBA{0xb3, 0x00, 0x00, 0xff},
			color.RGBA{0xe3, 0x4a, 0x33, 0xff},
			color.RGBA{0xfc, 0x8d, 0x59, 0xff},
			color.RGBA{0xb3, 0x00, 0x00, 0xff},
			color.RGBA{0xfe, 0xf0, 0xd9, 0xff},
		},
	},
	{
		"blues",
		[5]color.Color{
			color.RGBA{0x31, 0x82, 0xbd, 0xff},
			color.RGBA{0x6b, 0xae, 0xd6, 0xff},
			color.RGBA{0xbd, 0xd7, 0xe7, 0xff},
			color.RGBA{0xef, 0xf3, 0xff, 0xff},
			color.RGBA{0xef, 0xf3, 0xff, 0xff},
		},
	},
	{
		"greens",
		[5]color.Color{
			color.RGBA{0x00, 0x6d, 0x2c, 0xff},
			color.RGBA{0x31, 0xa3, 0x54, 0xff},
			color.RGBA{0x74, 0xc4, 0x76, 0xff},
			color.RGBA{0xba, 0xe4, 0xb3, 0xff},
			color.RGBA{0xed, 0xf8, 0xe9, 0xff},
		},
	},
	{
		"pinks",
		[5]color.Color{
			color.RGBA{0x98, 0x00, 0x43, 0xff},
			color.RGBA{0xdd, 0x1c, 0x77, 0xff},
			color.RGBA{0xdf, 0x65, 0xb0, 0xff},
			color.RGBA{0xd7, 0xb5, 0xd8, 0xff},
			color.RGBA{0xf1, 0xee, 0xf6, 0xff},
		},
	},
	{
		"magic_beans",
		[5]color.Color{
			color.RGBA{0x05, 0x71, 0xb0, 0xff},
			color.RGBA{0x92, 0xc5, 0xde, 0xff},
			color.RGBA{0xf7, 0xf7, 0xf7, 0xff},
			color.RGBA{0xf4, 0xa5, 0x82, 0xff},
			color.RGBA{0xca, 0x00, 0x20, 0xff},
		},
	},
	{
		"pink_to_green",
		[5]color.Color{
			color.RGBA{0x00, 0x88, 0x37, 0xff},
			color.RGBA{0x92, 0xdb, 0xa0, 0xff},
			color.RGBA{0xf7, 0xf7, 0xf7, 0xff},
			color.RGBA{0xc2, 0xa5, 0xcf, 0xff},
			color.RGBA{0x7b, 0x32, 0x94, 0xff},
		},
	},
	{
		"ropgw",
		[5]color.Color{
			color.RGBA{0xef, 0xf3, 0xe5, 0xff},
			color.RGBA{0xe5, 0xec, 0x8d, 0xff},
			color.RGBA{0xde, 0xcc, 0xd8, 0xff},
			color.RGBA{0xc6, 0xa2, 0x72, 0xff},
			color.RGBA{0xa6, 0x1f, 0x1b, 0xff},
		},
	},
}

// DrawLegend
func DrawLegend(w io.Writer, breaks []float64, scheme int, units string) error {
	if scheme > len(colorSchemes) {
		return fmt.Errorf("invalid scheme: %d", scheme)
	}
	rgba := image.NewRGBA(image.Rect(0, 0, legendWidth, legendHeight))
	draw.Draw(rgba, rgba.Bounds(), &image.Uniform{color.Black}, image.ZP, draw.Src)

	d := &font.Drawer{
		Dst:  rgba,
		Src:  image.NewUniform(color.White),
		Face: inconsolata.Regular8x16,
	}

	colors := colorSchemes[scheme].colors
	point := fixed.Point26_6{fixed.Int26_6(12 * 64), fixed.Int26_6(12 * 64)}
	d.Dot = point
	d.DrawString(fmt.Sprintf("Wind Speed(%s)", units))
	x := 10
	y := 30
	var low, high float64
	var s string
	for i, c := range colors {
		draw.Draw(rgba, image.Rect(x, y, x+30, y+1), &image.Uniform{c}, image.Pt(x, y), draw.Src)
		xx := x + 30
		for i := 0; i < 6; i++ {
			rgba.Set(xx-i, y-i, c)
			rgba.Set(xx-i, y+i, c)
		}
		xx = xx + 10
		yy := y + 6
		point := fixed.Point26_6{fixed.Int26_6(xx * 64), fixed.Int26_6(yy * 64)}
		d.Dot = point
		low = breaks[i]
		if i < len(colors)-1 {
			high = breaks[i+1]
			high -= 0.1
			s = fmt.Sprintf("%.1f-%.1f", low, high)
		} else {
			s = fmt.Sprintf("%.1f +", low)
		}
		d.DrawString(s)
		y += 30
	}

	// Save that RGBA image to disk.
	err := png.Encode(w, rgba)
	if err != nil {
		return err
	}
	return nil
}
