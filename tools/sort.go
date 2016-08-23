package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"sort"
	"strings"
)

const usage = `
sort [-i] input
`

// Sort does what `sort` does, but skips leading bash commented lines
func main() {
	inPlace := flag.Bool("i", false, "overwrite existing file")
	flag.Parse()
	args := flag.Args()
	if len(args) < 1 {
		fmt.Println(usage)
	}

	var comments []string
	var lines []string
	fin, err := os.Open(args[0])
	if err != nil {
		fmt.Printf("%s\n", err)
		os.Exit(1)
	}
	defer fin.Close()
	scanner := bufio.NewScanner(fin)
	for scanner.Scan() {
		fields := strings.Fields(scanner.Text())
		if len(fields) > 0 && len(fields[0]) > 0 && strings.HasPrefix(fields[0], "#") {
			comments = append(comments, scanner.Text())
		} else if len(fields) < 1 {
			continue
		} else {
			lines = append(lines, scanner.Text())
		}
	}
	if err := scanner.Err(); err != nil {
		fmt.Printf("%s\n", err)
		os.Exit(1)
	}
	sort.Strings(lines)
	all := append(comments, lines...)
	var fout *os.File
	if *inPlace {
		fin.Close()
		fout, err = os.Create(args[0])
		if err != nil {
			fmt.Printf("%s\n", err)
			os.Exit(1)
		}
		defer fout.Close()
	} else {
		fout = os.Stdout
	}
	for _, line := range all {
		fmt.Fprintf(fout, "%s\n", line)
	}
}
