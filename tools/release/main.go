// Command release emits shields.io endpoint JSON for the README
// release badge. The gh-pages pipeline publishes badge-release.json
// so the badge does not depend on shields.io's shared GitHub API
// token pool.
package main

import (
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
)

const cmdBadge = "badge"

var errUsage = errors.New("usage error")

func run(args []string, stdout, stderr io.Writer) error {
	if len(args) < 1 {
		usage(stderr)
		return errUsage
	}
	switch args[0] {
	case cmdBadge:
		return runBadge(args[1:], stdout, stderr)
	case "-h", "--help", "help":
		usage(stdout)
		return nil
	default:
		_, _ = fmt.Fprintf(stderr, "release: unknown subcommand %q\n\n", args[0])
		usage(stderr)
		return errUsage
	}
}

func usage(w io.Writer) {
	_, _ = fmt.Fprint(w, `release - gh-pages badge JSON for the README release shield.

Subcommands:
  badge  Emit shields.io endpoint JSON for the latest semver release tag.

Run "release badge -h" for flags.
`)
}

func flagSet(name string, stderr io.Writer) *flag.FlagSet {
	fs := flag.NewFlagSet(name, flag.ContinueOnError)
	fs.SetOutput(stderr)
	return fs
}

func main() {
	if err := run(os.Args[1:], os.Stdout, os.Stderr); err != nil {
		if errors.Is(err, errUsage) {
			os.Exit(2)
		}
		_, _ = fmt.Fprintf(os.Stderr, "release: %v\n", err)
		os.Exit(1)
	}
}
