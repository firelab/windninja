package thredds

import (
	"bytes"
	"testing"
	"time"
)

// Use mackay bounds
const (
	minX = -113.749693430469
	maxX = -113.463446144564
	minY = 43.7832152227745
	maxY = 44.0249023401036
)

func TestDownload(t *testing.T) {
	tests := []string{
		NAM_CONUS,
		NAM_ALASKA,
		GFS,
	}
	for _, model := range tests {
		//b, _ := os.Create(model + ".nc")
		//defer b.Close()
		b := &bytes.Buffer{}
		err := Download(b, model, minX, maxX, minY, maxY, time.Hour*6)
		if err != nil {
			t.Error(err)
		}
	}
}
