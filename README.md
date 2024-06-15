# ChiptunePractice
## Overview
This project aims to develop a synthesizer inspired by the iconic 8-bit sounds of the Nintendo
Entertainment System, focusing on recreating the retro sounds associated with vintage video games.
The synthesizer features three primary waveforms—pulse, triangle, and noise—each equipped with
unique settings. Additionally, it integrates sophisticated pitch modulation capabilities, including pitch
bend, vibrato, and an arpeggiator. It also features envelope control and two digital signal processing
(DSP) effects: bitcrusher and delay. With its comprehensive control parameters, the synthesizer serves
as a useful tool for both music production and sound effect generation.

## Waveforms
### 1. Pulse Wave
Due to the nature of value discontinuity, the pulse wave tends to suffer from significant aliasing issues.
To mitigate this, I incorporated the polyBLEP method to generate the pulse wave, introducing a
quadratic function at the value discontinuity boundaries to smooth it out. However, even with
polyBLEP, high pitches still encounter aliasing issues. As I lack basic knowledge of DSP, addressing this
issue has been challenging, thus the optimal performance range for the pulse wave remains limited.
One potential solution to explore could be additive synthesis using wavetables to store the values of
sine tones.
The NES's pulse wave channels offer 4 duty cycles: 12.5%, 25%, 50%, and 75% (explod2A03, 2012).
Since the 25% and 75% duty cycles sound identical due to inversion, this synthesizer provides only
three specific options: 12.5%, 25%, and 50% pulse widths. In addition to these fixed pulse width
selections, the synthesizer also includes a pulse width modulation module that allows for six modes
of pulse width variation with adjustable rates. 


### 2. Triangle Wave
Distinctive for its asymmetrical rise and slightly curved fall, the NES triangle wave is accurately
replicated here by modifications in the TriOsc class. This waveform is also quantized into 16 discrete
amplitudes, which adds even harmonics due to this irregularity (Montag, 2011), making it have distinct
high-frequency harmonics.
The triangle wave provides a distortion option. If distortion is chosen, the result is a "Pseudo Triangle
Wave" similar to that of the NES, created using a bit crusher to produce a 4-bit triangle wave. Without
distortion, the output is a smooth Triangle Wave. For the distortion effect, using a wavetable with prewritten
data for the 16 steps might be a better approach, as attempts to create a 16-step waveform
with a bitcrusher did not yield as clear a result as expected.

### 3. Noise
Unlike traditional noise synthesis, which involves random modulation of amplitude or frequency, the
NES synthesizes noise in predetermined sequences (Montag, 2011). To simulate this behaviour, I
utilized a wavetable synthesizer approach.
The noise wave provides a "Noisy Noise" selection. If it is chosen, the result is synthesized via a 3000-
sample wavetable filled with randomly generated 4-bit values. Otherwise, the synthesizer produces
genuine random white noise.


## Pitch Modulation Modules
In addition to waveform generation, this synthesizer incorporates pitch modulation modules to add
expression and dynamics to the sound.

### 1. Pitch Bend
The Pitch Bend module consists of two main parameters: initial pitch and time. The initial pitch defines
the difference from the input pitch at the note's onset, while the time parameter controls the duration
it takes to transition to the input pitch. This feature is particularly useful for creating sound effects or
adding quick frequency sweeps.
Currently, Pitch Bend can only be applied at the start of a note. Future updates may include the ability
to apply it at the end of a note as well, allowing users to control when the pitch bend begins,
potentially integrating a sustain parameter similar to the one in the Vibrato module.

### 2. Vibrato
The Vibrato module includes three parameters: speed, amount, and sustain. Speed controls the
frequency of the vibrato, amount controls the depth of the modulation, and sustain sets the delay
before vibrato kicks in.
Usually, we only need to add vibrato effect to sustained notes to make the listening experience less
monotonous. In such cases, you can set the sustain parameter to an appropriate time, so that vibrato
does not activate during short notes, only triggering within longer notes. Both the speed and amount
parameters are updated in real-time through the process function in the class, supporting automation
within a DAW for more dynamic and naturalistic vibrato effects.

### 3. Arpeggiator
The Arpeggiator module offers options for pattern, loop mode, and speed, allowing for diverse and
dynamic pitch sequences. The pattern parameter includes nine different modes, covering common
intervals, a few chord patterns, and a random mode that generates pitches within a range of -7 to +7
semitones.
Loop mode offers choices between 1 repeat, 1 octave, and 2 octaves, determining how the pattern is
repeated. The 1 repeat option simply executes the pattern sequence once and then holds the last note,
while the 1 octave and 2 octaves settings allow the pattern to loop over one or two octave ranges,
respectively.
Speed determines how quickly the pattern progresses. Fast-speed arpeggiators are widely used in 8-
bit game music.

## DSP
### 1. Bitcrusher
Primarily used to distort the triangle waveform, the bitcrusher is also available as a standalone effect
for user-defined timbre modification. This module has a very noticeable effect on the noise waveform,
enabling users to adjust the timbre as desired.

### 2. Delay
This module primarily emulates the NES's use of two pulse wave channels to create reverb/echo
effects. Additionally, the delay can also aid in the creation of sound effects, enhancing the
synthesizer’s versatility.


## Conclusion
Despite the challenges posed by waveform optimization and a steep learning curve in foundational
audio programming, this project has significantly advanced my understanding and capability in
synthesizer development. While further improvements in sound quality are necessary, the successful
implementation of various modules provides a solid foundation for future enhancements and learning
in the field of audio programming.

## Install instruction
For Mac, just paste the VST3/AU file into your plugin path. The default path should be:
VST3: /Library/Audio/Plug-Ins/VST3
AU: /Library/Audio/Plug-Ins/Components
