# An open-source - community built micro-inverter

## Manifesto 

The project goal are : 

- Having the first open source PV micro-inverter
- Finally a design easy to repair that is no longer a black box
- Redeem your data sovereignty, you will no longer depend on a proprietary cloud to follow your solar production and consumption. 

## Open, for everyone, forever

- This project is propulsed by its community. 
- The collective work generated is meant to remain open. 
- Design files are made with open source software - Namely kiCAD.

In that sense, the owntech foundation aims at protecting this technical common.

## Expected design crux 

The transformer `T1` is one of the core problems, we preferably would like to design the micro-inverter around a standard of the shelf transformer to enable easy replication. Manually wound transformer have wider parameters disparity which leads to non practical replications.

- Finding off the shelf high frequency transformers is hard
- They tend not to have the required clearance and voltage isolation class. 

## (ultra) Preliminary specifications

- 230V 50Hz AC grid code compliant
- 110V 60Hz maybe in the future by changing the transformer
- 450W max per module
- No invasive potting that prevents reparability
- No constraints on height at least for first prototypes - (no real need to have it super flat but nice to have) 
- Off the shelf transformer

## Standards and directives

### Application related standards

- IEC 62109: Safety requirements for photovoltaic inverters.
- IEC 61727: Requirements for photovoltaic (PV) systems to interface with the utility grid.
- IEC 62116: Test procedure for islanding prevention measures for utility-interconnected photovoltaic inverters.
- EN 50438: Requirements for the connection of micro-generators in parallel with public low-voltage distribution networks.
- EN 50549: Requirements for generating plants to be connected in parallel with distribution networks (superseeds DIN VDE 0126)

### Power electronics related standards 
- IEC-62477 Safety requirements for power electronic converter systems and equipment
- IEC-60664 Insulation coordination for equipment within low-voltage supply systems
- IEC-61000 Electromagnetic compatibility (EMC)

## Licence 

This project is licenced under **CERN-OHL-V2-S** open source hardware licence. Licence file can be found under `License/cern-ohl-v2-s`. 
 



