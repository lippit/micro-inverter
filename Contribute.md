![Team Banneer](Images/team_banneer.jpg)

# Contribute

This project is a technical common: open, repairable, auditable, and built by its community.  
Whether you bring careful eyes, lab time, code, documentation, or field experience — you’re welcome.

If you’re unsure where to start: open an issue, introduce yourself, and tell us what you’d like to work on.

---

## How to get involved

### 1) Pick a contribution path
Choose a topic below (Hardware / Firmware / Dataware). Each topic contains hands-on work items you can adopt.

### 2) Coordinate in public
Before starting large changes, please:
- Check existing issues / discussions
- Open a new issue describing your plan (scope + expected outcome)
- Share progress early (screenshots, measurements, logs)

This keeps effort aligned and avoids duplicated work.

### 3) Make a pull request
A good PR is small, reviewable, and documented:
- One clear purpose per PR
- Explain *what* changed and *why*
- Include screenshots for PCB/mechanical changes
- Include measurements / plots / logs when relevant
- Link the issue it addresses (e.g. `Fixes #123`)


---

## Contribution topics

## Hardware (KiCad / FreeCAD / lab work)

- Create a thermal model for the uVerter. 
- Explore how we could extend DCDC stage to more solar panels using interleaving.
- Explore how to create an optimized casing for the uVerter.

---

## Firmware (control, protection, bring-up)

- Add a hard real-time protection layer in `loop_critical_task()` that can force “cease to energize” (e.g., `shield.power.stop(ALL)` plus gate/relay disable) within grid-code limits, independent of control flow.
- Improve ride-through vs anti-islanding by tuning synchronization logic in `loop_critical_task()` (e.g., `is_net_synchronized`, `sync_counter`, `desync_counter`) and refining the `STARTUPMODE`/`POWERMODE` transitions in `loop_application_task()`.
- Implement deterministic grid-code behaviors with explicit timing using the state machine in `loop_application_task()` and setpoint/duty shaping in `loop_critical_task()` (e.g., `rate_limiter()` for ramps and `mode_asked` for reconnection delays).

---

## Dataware (local-first monitoring, APIs, UX)

- Wire an ESP32 on top of the spin board and implement a minimalistic WIFI monitoring using thingset protocol.
