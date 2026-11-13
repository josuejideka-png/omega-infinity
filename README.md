# OMEGA INFINITY V1.1

Synthe VST3 polyphonique (8 voix) pour Windows 64 bits, code source JUCE.

## Contenu
- 8 voix, oscillateur sine / saw / square avec PolyBLEP (anti-aliasing)
- Enveloppe ADSR complete, filtre passe-bas resonant
- Reverb avec dosage, 6 presets d'usine, pitch bend
- Sauvegarde et rappel de l'etat par l'hote

## Compiler sans rien installer (GitHub)
1. Creer un depot GitHub et y deposer tout ce dossier.
2. Onglet Actions > "Build VST3 (Windows)" > Run workflow.
3. Telecharger l'artefact OMEGA_INFINITY_VST3 une fois termine.
4. Copier le dossier OMEGA INFINITY.vst3 dans C:\Program Files\Common Files\VST3
5. Dans FL Studio : Options > Manage plugins > Find plugins.

## Non teste
Ce code n'a pas ete compile ni teste ici. Le premier build peut reveler des
erreurs a corriger : envoie-moi le journal et je corrige.

## Prochaines etapes
Module de voix echantillonnees, rack d'effets (delay, chorus, distorsion).
