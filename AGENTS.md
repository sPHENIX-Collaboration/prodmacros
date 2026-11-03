# sPHENIX production agent guidance

This is the shared agent guidance for `prodmacros` and `sphenixprod`. Keep the detailed production procedure in [Named_Productions.md](Named_Productions.md); keep this file focused on cross-repository decisions and pitfalls. The `sphenixprod/AGENTS.md` file imports this guidance and adds toolchain-specific orientation.

## Where to look

- `prodmacros/Named_Productions.md` is the operational handbook for creating, activating, and cleaning productions. `prodmacros/run*/` contains dataset templates; `prodmacros/active_productions.txt` is the master pilot list. In the standard `Production2026` layout, the top-level `active_productions.txt` symlinks to that list.
- `sphenixprod/README.md` explains the toolchain. Check the actual `sphenixprod` code for current behavior: `production_control.py` interprets pilots, `create_submission.py` creates jobs, `sphenixprodrules.py` parses rules, and `stageout.sh` handles outputs. Do not infer supported placeholders or flags from a template alone.
- Production instances are separate repositories under `Production2026`. Their `thisProd/` files, branch, tag, and pilot are the source for that instance. A template edit does not update an existing instance automatically.

## Production changes

- Inspect the working tree and active pilot before editing. Preserve unrelated local changes and distinguish template edits from instance edits. Make general macro changes in `prodmacros`, then deliberately forward them to affected instances.
- Use the dataset, type, physics mode, build, DB tag, and version to form names. Remove periods from the build only in names and triplets; keep the period in rule YAML `build` (for example, `ana.581`). Set numeric `version`, and check each downstream `intriplet` against the upstream rule's actual output triplet. Steps may use different DB tags.
- Place an instance's rule and pilot under `thisProd/rules/` and `thisProd/pilots/`. Point `configbase` at its absolute `thisProd/rules` path and use an absolute runlist path. Set the requested submit host in the pilot. Verify `submit`, `dstspider`, and default `histspider` behavior for each stage; turning a pilot stage off does not cancel already queued jobs.
- Preserve unrelated entries in the master active list. Before activating a production, parse its YAML, check file paths and rule names, search for stale placeholders and triplets, and run `production_control.py --dryrun --hostname <host>` when practical. Verify shell syntax for edited wrappers.
- Record production-instance changes on that instance's branch. Use a new annotated tag for an update rather than moving a published tag. Check the branch's upstream before pushing. Do not publish a branch or tag unless the user has authorized the push.
- For a direct single-rule preview, use `create_submission.py --config <rule.yaml> --rulename <rule> --runs <run> --dryrun`. `--andgo` submits jobs; consult `sphenixprod/README.md` for full command usage.

## On the shared sPHENIX host

- There is no testbed database switch on this production host. Commands that submit jobs or update, register, or delete production data can affect the real databases and output areas. Use each command's documented `-n`/`--dryrun` option for a preview; check its behavior before a live run.
- After sourcing `sphenixprod/this_sphenixprod.sh`, `ODBCINI` is `./.odbc.ini`. Run database commands from a directory with the intended `.odbc.ini`, or set `ODBCINI` explicitly.
- The sandbox `bwrap` helper may fail to start here; use targeted shell edits with the required filesystem escalation instead of `apply_patch` when that happens.
- Ask for permission before executing any `rm` command. Never add a persistent approval rule for destructive commands such as `rm`.
