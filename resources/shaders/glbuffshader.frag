#ifdef GL_ES
    precision highp int;
    precision highp float;
    precision highp sampler2D;
#endif

in vec2 texuv;
flat in int offset;
flat in int paletteid;

out vec4 FragColour;

uniform sampler2D texture0;
uniform sampler2D paltex2D;

void main() {
    vec4 fragcol = texture(texture0, texuv);
    int index = int(fragcol.r * 255.0);

    // So, the way this works.
    //
    // Icons are made using a gradient of 64 different colors. E.g. for a feather (feather fall spell) the
    // gradient literally goes along the length of the feather. The border of the icon is single color.
    //
    // Then, we have a palette that has a colored gradient that's 64 colors long, from lighter to darker colors.
    // We extend this gradient by transposing it and adding it to itself, so we get a gradient 126 colors long.
    // Why 126 colors? Because we don't duplicate the starting & ending colors. The nice thing about the
    // resulting gradient is that it's looped.
    //
    // What we do next can be visualized like this:
    //
    // Color gradient: |-------------------------------------------------------------------------------|
    // Palette:                      |-------------------------------------|
    //
    // We just move the palette mapping along the gradient, wrapping around as necessary. This results in nice
    // animation.
    if (index <= 63) {
        index = (index + offset) % (2 * 63);
        if (index >= 63)
            index = 2 * 63 - index;
    }

    vec4 newcol = vec4(texelFetch(paltex2D, ivec2(index, paletteid), 0));

    fragcol = vec4(newcol.r, newcol.g, newcol.b, 1.0);

    FragColour =  fragcol;
}
