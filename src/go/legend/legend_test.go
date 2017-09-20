package legend

import (
	"os"
	"path/filepath"
	"testing"
)

func TestColorSchemes(t *testing.T) {
	pngFiles, _ := filepath.Glob("./test/*.png")
	for _, png := range pngFiles {
		os.Remove(png)
	}
	breaks := []float64{0.0, 1.0, 2.0, 3.0, 4.0, 5.0}
	for i, cs := range colorSchemes {
		p := filepath.Join("test", cs.name+".png")
		fout, err := os.Create(p)
		if err != nil {
			t.Fatal(err)
		}
		err = DrawLegend(fout, breaks, i, "mph")
		if err != nil {
			t.Error(err)
		}
	}
}
