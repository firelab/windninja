package legend

import (
	"image"
	"image/color"
	"image/draw"
	"image/png"
	"io"

	"golang.org/x/image/font"
	"golang.org/x/image/font/inconsolata"
	"golang.org/x/image/math/fixed"
)

// Banner
func Banner(w io.Writer, width, height int, message string) error {
	width = 140
	height = 180
	rgba := image.NewRGBA(image.Rect(0, 0, width, height))
	draw.Draw(rgba, rgba.Bounds(), &image.Uniform{color.Black}, image.ZP, draw.Src)
	d := &font.Drawer{
		Dst:  rgba,
		Src:  image.NewUniform(color.White),
		Face: inconsolata.Regular8x16,
	}
	point := fixed.Point26_6{fixed.Int26_6(12 * 64), fixed.Int26_6(12 * 64)}
	d.Dot = point
	d.DrawString(message)
	err := png.Encode(w, rgba)
	if err != nil {
		return err
	}
	return nil
}
