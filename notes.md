# Rotational Physics — Working Notes

Everything needed to add rotation to phyX, in the order the ideas actually depend on
each other. Written to be re-read after a long break.

---

## 0. The map

You already have linear motion working. Rotation is the same five ideas with different
types. Keep this table in your head — every confusion later is a place where the mirror
almost holds but not quite.

| | Linear (done) | Angular (to build) |
|---|---|---|
| Where is it | `Position` — `Vec3` | `Orientation` — `Quaternion` |
| How much motion | `Momentum` **p** — **state** | `AngularMomentum` **L** — **state** |
| Resistance to change | `Mass` *m* — one number, constant | Inertia **I** — three numbers, *and it turns with the body* |
| Speed | **v** = **p**/*m* — derived | **ω** = **I**⁻¹**L** — derived |
| What changes it | force **F** | torque **τ** = **r** × **F** |
| Integrate | `p += F·dt` | `L += τ·dt` |
| Then | `x += v·dt` | `q += ½·ω⊗q·dt`, then normalize |

Two rows are where the mirror breaks, and those two are the whole difficulty:

- **Resistance is not one number.** A box is harder to spin end-over-end than about its
  long axis. One scalar can't say that.
- **Orientation is not a vector.** You can't add two orientations. Rotations don't
  commute — order changes the answer.

Everything below is unpacking those two.

---

## 1. Angular velocity ω — one vector, not three

The first instinct is that spin needs three numbers: "spinning this fast about x, this
fast about y, this fast about z." Drop that. **At any instant a rigid body spins about
exactly one axis.** So ω is a single vector:

- **direction** = the axis it's spinning about (right-hand rule: curl your fingers the
  way it turns, your thumb is ω)
- **length** = how fast, in radians per second

The three components are just that one arrow written in coordinates.

### What ω is actually for

The reason ω matters is this formula, which is the bridge between rotation and everything
you already have:

> **velocity of a point on the body = v_cm + ω × r**
>
> where **r** is the offset from the center of mass to that point.

<svg viewBox="0 0 470 300" width="100%" style="max-width:470px;height:auto" role="img" aria-label="Angular velocity omega out of the page, with radius vector r and resulting surface velocity v perpendicular to r">
  <defs>
    <marker id="f1-o" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#d19a66"/></marker>
    <marker id="f1-g" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#98c379"/></marker>
    <marker id="f1-b" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto"><path d="M0,0 L8,3 L0,6 z" fill="#61afef"/></marker>
  </defs>
  <circle cx="170" cy="150" r="100" fill="none" stroke="#888" stroke-width="1.2" stroke-dasharray="4 4"/>
  <path d="M 270 150 A 100 100 0 0 0 240 79" fill="none" stroke="#61afef" stroke-width="1.6" marker-end="url(#f1-b)" opacity="0.75"/>
  <circle cx="170" cy="150" r="9" fill="none" stroke="#61afef" stroke-width="2"/>
  <circle cx="170" cy="150" r="3.2" fill="#61afef"/>
  <text x="146" y="176" font-family="ui-sans-serif,system-ui,sans-serif" font-size="15" font-weight="600" fill="#61afef">ω</text>
  <text x="112" y="194" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">out of the page</text>
  <line x1="170" y1="150" x2="235" y2="85" stroke="#d19a66" stroke-width="2.2" marker-end="url(#f1-o)"/>
  <text x="212" y="132" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#d19a66">r</text>
  <circle cx="241" cy="79" r="4" fill="#d19a66"/>
  <line x1="241" y1="79" x2="199" y2="37" stroke="#98c379" stroke-width="2.2" marker-end="url(#f1-g)"/>
  <text x="150" y="30" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#98c379">v = ω × r</text>
  <line x1="170" y1="150" x2="201" y2="119" stroke="#d19a66" stroke-width="1.6" opacity="0.55" marker-end="url(#f1-o)"/>
  <circle cx="205" cy="115" r="3.2" fill="#d19a66" opacity="0.6"/>
  <line x1="205" y1="115" x2="184" y2="94" stroke="#98c379" stroke-width="1.6" opacity="0.55" marker-end="url(#f1-g)"/>
  <text x="300" y="120" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">|v| = |ω| × distance</text>
  <text x="300" y="140" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">from the axis.</text>
  <text x="300" y="170" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">Twice as far out</text>
  <text x="300" y="188" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">⇒ twice as fast.</text>
  <text x="300" y="216" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">On the axis itself,</text>
  <text x="300" y="234" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">r = 0 ⇒ v = 0.</text>
</svg>

Why a cross product? A point being spun moves **perpendicular** to both the axis and its
offset from the axis, and its speed grows with how far out it sits. "Perpendicular to
both, scaled by both" *is* the definition of the cross product. Nothing deeper.

**This one formula is why rotation changes your collision code.** A spinning ball touching
the ground has a contact point that is *moving sideways* even though the ball's center
isn't. That sideways motion is what friction grabs. Without `ω × r`, nothing can ever roll.

---

## 2. Inertia — why one number isn't enough

Mass answers *"how hard is it to push this?"* — one number, same in every direction.

Inertia answers *"how hard is it to spin this?"* — and the honest answer is **it depends
which way you spin it.**

<svg viewBox="0 0 500 235" width="100%" style="max-width:500px;height:auto" role="img" aria-label="A rod spun about its long axis is easy; the same rod spun end over end is hard">
  <defs>
    <marker id="f2-o" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto"><path d="M0,0 L8,3 L0,6 z" fill="#d19a66"/></marker>
  </defs>
  <line x1="20" y1="112" x2="222" y2="112" stroke="#61afef" stroke-width="1.6" stroke-dasharray="6 4"/>
  <rect x="52" y="98" width="140" height="28" rx="14" fill="none" stroke="#888" stroke-width="1.6"/>
  <circle cx="80" cy="105" r="3" fill="#888"/><circle cx="110" cy="119" r="3" fill="#888"/>
  <circle cx="140" cy="104" r="3" fill="#888"/><circle cx="168" cy="118" r="3" fill="#888"/>
  <line x1="80" y1="112" x2="80" y2="105" stroke="#d19a66" stroke-width="1.6"/>
  <line x1="140" y1="112" x2="140" y2="104" stroke="#d19a66" stroke-width="1.6"/>
  <text x="40" y="62" font-family="ui-sans-serif,system-ui,sans-serif" font-size="13" font-weight="600" fill="#98c379">EASY — small I</text>
  <text x="40" y="82" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">every bit of mass sits</text>
  <text x="40" y="97" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">right next to the axis</text>
  <text x="60" y="158" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#61afef">spin about the long axis</text>
  <line x1="250" y1="30" x2="250" y2="215" stroke="#888" stroke-width="1" stroke-dasharray="3 5" opacity="0.5"/>
  <line x1="368" y1="42" x2="368" y2="190" stroke="#61afef" stroke-width="1.6" stroke-dasharray="6 4"/>
  <rect x="288" y="98" width="160" height="28" rx="14" fill="none" stroke="#888" stroke-width="1.6"/>
  <circle cx="300" cy="112" r="3.4" fill="#888"/><circle cx="330" cy="112" r="3.4" fill="#888"/>
  <circle cx="406" cy="112" r="3.4" fill="#888"/><circle cx="436" cy="112" r="3.4" fill="#888"/>
  <line x1="368" y1="140" x2="300" y2="140" stroke="#d19a66" stroke-width="1.6" marker-end="url(#f2-o)"/>
  <line x1="368" y1="140" x2="436" y2="140" stroke="#d19a66" stroke-width="1.6" marker-end="url(#f2-o)"/>
  <text x="330" y="158" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#d19a66">r⊥</text>
  <text x="286" y="62" font-family="ui-sans-serif,system-ui,sans-serif" font-size="13" font-weight="600" fill="#e06c75">HARD — large I</text>
  <text x="286" y="82" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">the mass sits far out,</text>
  <text x="286" y="97" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">and distance is squared</text>
  <text x="308" y="182" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#61afef">spin end-over-end</text>
  <text x="126" y="220" font-family="ui-sans-serif,system-ui,sans-serif" font-size="13.5" fill="#888">I = Σ m · r⊥²    — same rod, same mass, very different I</text>
</svg>

Take a pencil. Spin it about its long axis — trivial. Now flip it end over end — noticeably
harder. **Same object, same mass, different resistance.** So inertia needs at least three
numbers: one per axis.

The formula is `I = Σ m·r⊥²` — add up each bit of mass times its **squared** distance from
the axis. The square is why it's so lopsided: move mass twice as far out and it resists
four times as much. It's also why a hollow tube is much harder to spin than a solid rod of
the same weight.

### The tensor

In full generality inertia is a **3×3 symmetric matrix** (a *tensor*), because spinning
about one axis can produce angular momentum leaning toward another. But there's a shortcut
that saves you: **every rigid body has three special perpendicular axes — its principal
axes — where the off-diagonal terms vanish.** Line your body frame up with those and I is
just three numbers on a diagonal:

```
        [ I₁  0   0  ]
I_body = [ 0   I₂  0  ]     →  store only (I₁, I₂, I₃)
        [ 0   0   I₃ ]
```

For a symmetric shape those axes are the obvious ones, so this is free:

| Shape | Inertia |
|---|---|
| Solid sphere, radius *r* | `I₁ = I₂ = I₃ = (2/5)·m·r²` |
| Box, full sides *a,b,c* | `I₁ = (1/12)m(b²+c²)`, `I₂ = (1/12)m(a²+c²)`, `I₃ = (1/12)m(a²+b²)` |

Store the **reciprocals** (`1/I₁, 1/I₂, 1/I₃`) — same trick as `inverseMass`. You only ever
divide by inertia, never multiply, and a zero reciprocal cleanly means "can't be spun."

**The catch that drives everything in section 7:** these three numbers are constant *in the
body's own frame*. But the body turns, so in **world** coordinates the tensor changes every
single frame. That's the entire complication of rotational dynamics.

---

## 3. Angular momentum L — and the punchline

```
L = I ω
```

Mirror of `p = m v`. But look what the previous section just did to it.

For linear motion, *m* is a scalar, so **p and v always point the same way.** Always. You
can use either as your state and nothing changes.

For rotation, **I is different along different axes.** So `I ω` doesn't just rescale ω — it
stretches one component more than another, which **tilts the result.**

> ### L and ω generally point in *different directions*.

This is the single most important fact in rotational physics, and the one with no linear
analogue at all.

<svg viewBox="0 0 540 290" width="100%" style="max-width:540px;height:auto" role="img" aria-label="For a box, angular momentum L leans toward the hard axis away from omega; for a sphere they are parallel">
  <defs>
    <marker id="f3-b" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#61afef"/></marker>
    <marker id="f3-r" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#e06c75"/></marker>
  </defs>
  <line x1="140" y1="230" x2="330" y2="230" stroke="#888" stroke-width="1.2" stroke-dasharray="4 4"/>
  <line x1="140" y1="230" x2="140" y2="52" stroke="#888" stroke-width="1.2" stroke-dasharray="4 4"/>
  <text x="228" y="250" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">easy axis (small I)</text>
  <text x="150" y="48" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">hard axis (large I)</text>
  <rect x="112" y="200" width="56" height="34" rx="4" fill="none" stroke="#888" stroke-width="1.4" opacity="0.6"/>
  <line x1="140" y1="230" x2="238" y2="132" stroke="#61afef" stroke-width="2.4" marker-end="url(#f3-b)"/>
  <text x="244" y="132" font-family="ui-sans-serif,system-ui,sans-serif" font-size="15" font-weight="600" fill="#61afef">ω</text>
  <text x="244" y="148" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">tilted 45°</text>
  <line x1="140" y1="230" x2="184" y2="98" stroke="#e06c75" stroke-width="2.4" marker-end="url(#f3-r)"/>
  <text x="190" y="94" font-family="ui-sans-serif,system-ui,sans-serif" font-size="15" font-weight="600" fill="#e06c75">L</text>
  <text x="190" y="110" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">pulled toward hard axis</text>
  <path d="M 196 174 A 74 74 0 0 0 165 158" fill="none" stroke="#c678dd" stroke-width="1.5"/>
  <text x="196" y="196" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#c678dd">they disagree</text>
  <text x="112" y="272" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#888">BOX — I differs by axis</text>
  <line x1="372" y1="30" x2="372" y2="266" stroke="#888" stroke-width="1" stroke-dasharray="3 5" opacity="0.4"/>
  <circle cx="450" cy="230" r="30" fill="none" stroke="#888" stroke-width="1.4" opacity="0.6"/>
  <line x1="450" y1="230" x2="496" y2="140" stroke="#e06c75" stroke-width="4.5" marker-end="url(#f3-r)" opacity="0.65"/>
  <line x1="450" y1="230" x2="488" y2="156" stroke="#61afef" stroke-width="2.2" marker-end="url(#f3-b)"/>
  <text x="496" y="132" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#e06c75">L</text>
  <text x="452" y="140" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#61afef">ω</text>
  <text x="396" y="272" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#888">SPHERE — I equal everywhere</text>
  <text x="396" y="288" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">L ∥ ω always</text>
</svg>

Spin a box about an axis tilted between its easy and hard directions. The hard direction
contributes more to L, so **L leans toward the hard axis** while ω stays put. For a sphere,
I is the same in every direction, so the stretch is uniform and L stays parallel to ω —
which is exactly why spheres are the boring case.

### Why L is the state variable

Now the payoff, and the reason your momentum refactor was the right move.

With **no torque, L is conserved** — it's frozen in world space, exactly like linear
momentum. `ω` is *not* conserved. So:

- Store **L** → the update is `L += τ·dt`. That's it. Clean, exact, mirrors `p += F·dt`.
- Store **ω** → you'd have to integrate Euler's equations, complete with a `ω × (Iω)`
  correction term, because ω changes *even when nothing is pushing the body*.

Store the conserved thing. Derive the rest. That's the rule, and it's why `Recalculate()`
exists in your code.

### The free bonus: tumbling

Here's the visible consequence. Throw a box with no torque on it:

- **L is constant** — locked in world space, never moves.
- **ω is not** — because the body turns, the world-space inertia turns with it, so
  `ω = I⁻¹L` keeps changing even though L doesn't.

The spin axis wanders while the angular momentum sits still. That's **tumbling**, and you
get it *for free* — you never write a line of code for it. It's the reason a thrown phone
flips chaotically end over end but a thrown ball doesn't. (Spin a box about its *middle*
principal axis and it will periodically flip over on its own — the Dzhanibekov effect. Pure
consequence of `L = Iω`, no forces involved.)

**This is your test for section 7.** If your box tumbles and your sphere spins evenly, the
inertia code is correct.

---

## 4. Torque τ — the thing that changes L

```
τ = r × F        and        τ = dL/dt
```

Mirror of `F = dp/dt`. **r** is the lever arm: from the center of mass to where the force is
applied.

<svg viewBox="0 0 500 250" width="100%" style="max-width:500px;height:auto" role="img" aria-label="Torque equals r cross F; only the component of F perpendicular to the lever arm produces torque">
  <defs>
    <marker id="f4-o" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#d19a66"/></marker>
    <marker id="f4-r" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#e06c75"/></marker>
    <marker id="f4-g" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#98c379"/></marker>
    <marker id="f4-x" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#888"/></marker>
  </defs>
  <circle cx="105" cy="155" r="30" fill="none" stroke="#888" stroke-width="1.5"/>
  <circle cx="105" cy="155" r="3.5" fill="#888"/>
  <circle cx="105" cy="155" r="11" fill="none" stroke="#c678dd" stroke-width="1.8"/>
  <line x1="97.5" y1="147.5" x2="112.5" y2="162.5" stroke="#c678dd" stroke-width="1.8"/>
  <line x1="112.5" y1="147.5" x2="97.5" y2="162.5" stroke="#c678dd" stroke-width="1.8"/>
  <text x="72" y="200" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#c678dd">τ</text>
  <text x="52" y="218" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">into the page</text>
  <line x1="105" y1="155" x2="292" y2="96" stroke="#d19a66" stroke-width="2.2" marker-end="url(#f4-o)"/>
  <text x="188" y="112" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#d19a66">r</text>
  <circle cx="296" cy="94" r="4" fill="#d19a66"/>
  <line x1="296" y1="94" x2="352" y2="158" stroke="#e06c75" stroke-width="2.4" marker-end="url(#f4-r)"/>
  <text x="356" y="164" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#e06c75">F</text>
  <line x1="296" y1="94" x2="326" y2="84" stroke="#888" stroke-width="1.8" stroke-dasharray="4 3" marker-end="url(#f4-x)"/>
  <text x="330" y="78" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">F∥ — wasted</text>
  <line x1="296" y1="94" x2="318" y2="164" stroke="#98c379" stroke-width="2" stroke-dasharray="4 3" marker-end="url(#f4-g)"/>
  <text x="272" y="180" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#98c379">F⊥ — all the torque</text>
  <text x="378" y="42" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">Pulling straight out</text>
  <text x="378" y="59" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">along the lever does</text>
  <text x="378" y="76" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">nothing. Only the</text>
  <text x="378" y="93" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">sideways part turns it.</text>
  <text x="378" y="120" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">Longer lever ⇒ more</text>
  <text x="378" y="137" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">torque, same force.</text>
  <text x="60" y="242" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12.5" fill="#888">Force through the center: r = 0 ⇒ τ = 0. This is why gravity never spins anything.</text>
</svg>

Everything you already know about wrenches is in that cross product: a longer handle gives
more torque, and pulling straight along the handle does nothing at all.

**The consequence you must not miss:** gravity acts at the center of mass, so `r = 0`, so
`τ = 0`. **Gravity never changes a body's spin.** A thrown ball's rotation is constant the
entire time it's in the air. This is a great first thing to verify — turn on rotation, drop
a body, and its spin must not drift by even a little.

So all torque in a rigid-body engine comes from **contact** — forces applied at a point on
the surface, where `r ≠ 0`. Which is why rotation and collision response are really the same
feature.

---

## 5. Orientation — why it needs quaternions

Position is easy: three numbers, and to combine two moves you just add them. `A + B = B + A`.

Orientation is not like that, and here's the proof:

<svg viewBox="0 0 620 320" width="100%" style="max-width:620px;height:auto" role="img" aria-label="Two 90 degree rotations applied in opposite orders produce different final orientations">
  <defs>
    <marker id="f5-x" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto"><path d="M0,0 L8,3 L0,6 z" fill="#e06c75"/></marker>
    <marker id="f5-y" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto"><path d="M0,0 L8,3 L0,6 z" fill="#98c379"/></marker>
    <marker id="f5-z" markerWidth="8" markerHeight="6" refX="7" refY="3" orient="auto"><path d="M0,0 L8,3 L0,6 z" fill="#61afef"/></marker>
    <marker id="f5-a" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#888"/></marker>
  </defs>
  <text x="24" y="26" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#888">Body axes: X red, Y green, Z blue.  ⊙ = toward you,  ⊗ = away from you.</text>
  <line x1="79" y1="105" x2="112" y2="105" stroke="#e06c75" stroke-width="2" marker-end="url(#f5-x)"/>
  <text x="117" y="109" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <line x1="70" y1="96" x2="70" y2="66" stroke="#98c379" stroke-width="2" marker-end="url(#f5-y)"/>
  <text x="64" y="59" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <circle cx="70" cy="105" r="7.5" fill="none" stroke="#61afef" stroke-width="1.8"/><circle cx="70" cy="105" r="2.6" fill="#61afef"/>
  <text x="42" y="130" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <text x="44" y="150" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">start</text>
  <line x1="145" y1="105" x2="205" y2="105" stroke="#888" stroke-width="1.4" marker-end="url(#f5-a)"/>
  <text x="140" y="94" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">90° about X</text>
  <line x1="259" y1="105" x2="292" y2="105" stroke="#e06c75" stroke-width="2" marker-end="url(#f5-x)"/>
  <text x="297" y="109" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <circle cx="250" cy="105" r="7.5" fill="none" stroke="#98c379" stroke-width="1.8"/><circle cx="250" cy="105" r="2.6" fill="#98c379"/>
  <text x="222" y="130" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <line x1="250" y1="114" x2="250" y2="144" stroke="#61afef" stroke-width="2" marker-end="url(#f5-z)"/>
  <text x="244" y="160" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <line x1="325" y1="105" x2="385" y2="105" stroke="#888" stroke-width="1.4" marker-end="url(#f5-a)"/>
  <text x="320" y="94" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">90° about Y</text>
  <circle cx="430" cy="105" r="7.5" fill="none" stroke="#e06c75" stroke-width="1.8"/>
  <line x1="424.7" y1="99.7" x2="435.3" y2="110.3" stroke="#e06c75" stroke-width="1.8"/>
  <line x1="435.3" y1="99.7" x2="424.7" y2="110.3" stroke="#e06c75" stroke-width="1.8"/>
  <text x="402" y="130" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <line x1="439" y1="105" x2="472" y2="105" stroke="#98c379" stroke-width="2" marker-end="url(#f5-y)"/>
  <text x="477" y="109" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <line x1="430" y1="114" x2="430" y2="144" stroke="#61afef" stroke-width="2" marker-end="url(#f5-z)"/>
  <text x="424" y="160" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <line x1="79" y1="240" x2="112" y2="240" stroke="#e06c75" stroke-width="2" marker-end="url(#f5-x)"/>
  <text x="117" y="244" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <line x1="70" y1="231" x2="70" y2="201" stroke="#98c379" stroke-width="2" marker-end="url(#f5-y)"/>
  <text x="64" y="194" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <circle cx="70" cy="240" r="7.5" fill="none" stroke="#61afef" stroke-width="1.8"/><circle cx="70" cy="240" r="2.6" fill="#61afef"/>
  <text x="42" y="265" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <text x="44" y="285" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">start</text>
  <line x1="145" y1="240" x2="205" y2="240" stroke="#888" stroke-width="1.4" marker-end="url(#f5-a)"/>
  <text x="140" y="229" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">90° about Y</text>
  <circle cx="250" cy="240" r="7.5" fill="none" stroke="#e06c75" stroke-width="1.8"/>
  <line x1="244.7" y1="234.7" x2="255.3" y2="245.3" stroke="#e06c75" stroke-width="1.8"/>
  <line x1="255.3" y1="234.7" x2="244.7" y2="245.3" stroke="#e06c75" stroke-width="1.8"/>
  <text x="222" y="265" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <line x1="250" y1="231" x2="250" y2="201" stroke="#98c379" stroke-width="2" marker-end="url(#f5-y)"/>
  <text x="244" y="194" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <line x1="259" y1="240" x2="292" y2="240" stroke="#61afef" stroke-width="2" marker-end="url(#f5-z)"/>
  <text x="297" y="244" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <line x1="325" y1="240" x2="385" y2="240" stroke="#888" stroke-width="1.4" marker-end="url(#f5-a)"/>
  <text x="320" y="229" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10.5" fill="#888">90° about X</text>
  <line x1="430" y1="231" x2="430" y2="201" stroke="#e06c75" stroke-width="2" marker-end="url(#f5-x)"/>
  <text x="424" y="194" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#e06c75">X</text>
  <circle cx="430" cy="240" r="7.5" fill="none" stroke="#98c379" stroke-width="1.8"/><circle cx="430" cy="240" r="2.6" fill="#98c379"/>
  <text x="402" y="265" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#98c379">Y</text>
  <line x1="439" y1="240" x2="472" y2="240" stroke="#61afef" stroke-width="2" marker-end="url(#f5-z)"/>
  <text x="477" y="244" font-family="ui-sans-serif,system-ui,sans-serif" font-size="12" font-weight="600" fill="#61afef">Z</text>
  <text x="530" y="168" font-family="ui-sans-serif,system-ui,sans-serif" font-size="30" font-weight="600" fill="#c678dd">≠</text>
  <path d="M 505 108 L 517 108 L 517 228 L 505 228" fill="none" stroke="#c678dd" stroke-width="1.4"/>
  <text x="556" y="152" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">same two</text>
  <text x="556" y="168" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">rotations,</text>
  <text x="556" y="184" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">swapped order</text>
</svg>

Try it with your phone right now. Rotate it 90° about X then 90° about Y; reset; do Y then
X. **Different final orientation.** Rotations do not commute.

That kills the naive representations:

- **Three angles you add up (Euler angles)?** Addition commutes; rotation doesn't. So the
  representation is lying. It also has *gimbal lock* — at certain orientations two of your
  three axes collapse onto each other and you silently lose a degree of freedom.
- **A rotation matrix?** Honest and correct, but 9 numbers with **6 hidden constraints**
  (three unit-length columns, three perpendicularity conditions). Nudge it during
  integration and it stops being a rotation — it starts shearing your object.

### The quaternion

A quaternion stores an **axis and an angle**, but with the angle halved:

```
w      = cos(θ/2)          ← one number
vector = axis · sin(θ/2)   ← three numbers
```

4 numbers, **1 constraint** (`w² + |v|² = 1`), 3 real degrees of freedom. Exactly what a
rotation needs, with the cheapest possible constraint to maintain: one `sqrt` and four
divides. That is the entire reason quaternions won.

Three things to internalize:

- **Applying it is a sandwich:** `v' = q ⊗ (0,v) ⊗ q*`. The vector goes in as a quaternion
  with zero scalar part, and comes back out the same way.
- **The half-angle is because of the sandwich.** `q` appears *twice*, contributing θ/2 each
  time. Store the half, apply it twice, get θ. That's all the ½ means — and it's the same ½
  that shows up in the integration formula below, for the same reason.
- **Composing rotations is multiplication**, `q_total = q₂ ⊗ q₁`, and quaternion multiply is
  **not commutative** — which is exactly right, because neither is rotation. The algebra
  matches the physics.

One quirk worth knowing so it doesn't confuse you in a debugger: `q` and `−q` describe the
*same* rotation. Both are valid; don't panic when a sign flips.

---

## 6. Putting it together — one frame of simulation

<svg viewBox="0 0 640 320" width="100%" style="max-width:640px;height:auto" role="img" aria-label="Side by side pipelines: linear force to momentum to velocity to position, and angular torque to angular momentum to angular velocity to orientation">
  <defs>
    <marker id="f6-a" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#888"/></marker>
    <marker id="f6-p" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#c678dd"/></marker>
  </defs>
  <text x="25" y="28" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" font-weight="600" fill="#888" letter-spacing="1.2">LINEAR</text>
  <rect x="25" y="38" width="118" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.4"/>
  <text x="84" y="69" text-anchor="middle" font-family="ui-monospace,monospace" font-size="13" fill="#888">F</text>
  <line x1="143" y1="64" x2="178" y2="64" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="182" y="38" width="118" height="52" rx="7" fill="none" stroke="#61afef" stroke-width="2"/>
  <text x="241" y="62" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12.5" fill="#61afef">p += F·dt</text>
  <text x="241" y="79" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">STATE</text>
  <line x1="300" y1="64" x2="335" y2="64" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="339" y="38" width="118" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.4" stroke-dasharray="5 3"/>
  <text x="398" y="62" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12.5" fill="#888">v = p / m</text>
  <text x="398" y="79" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">derived</text>
  <line x1="457" y1="64" x2="492" y2="64" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="496" y="38" width="118" height="52" rx="7" fill="none" stroke="#61afef" stroke-width="2"/>
  <text x="555" y="62" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12.5" fill="#61afef">x += v·dt</text>
  <text x="555" y="79" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">STATE</text>
  <text x="25" y="146" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" font-weight="600" fill="#888" letter-spacing="1.2">ANGULAR</text>
  <rect x="25" y="156" width="118" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.4"/>
  <text x="84" y="187" text-anchor="middle" font-family="ui-monospace,monospace" font-size="13" fill="#888">τ = r × F</text>
  <line x1="143" y1="182" x2="178" y2="182" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="182" y="156" width="118" height="52" rx="7" fill="none" stroke="#e06c75" stroke-width="2"/>
  <text x="241" y="180" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12.5" fill="#e06c75">L += τ·dt</text>
  <text x="241" y="197" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">STATE</text>
  <line x1="300" y1="182" x2="335" y2="182" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="339" y="156" width="118" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.4" stroke-dasharray="5 3"/>
  <text x="398" y="180" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12.5" fill="#888">ω = I⁻¹ L</text>
  <text x="398" y="197" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">derived</text>
  <line x1="457" y1="182" x2="492" y2="182" stroke="#888" stroke-width="1.4" marker-end="url(#f6-a)"/>
  <rect x="496" y="156" width="118" height="52" rx="7" fill="none" stroke="#e06c75" stroke-width="2"/>
  <text x="555" y="180" text-anchor="middle" font-family="ui-monospace,monospace" font-size="12" fill="#e06c75">q += ½ω⊗q·dt</text>
  <text x="555" y="197" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="9.5" fill="#888">STATE — then normalize</text>
  <path d="M 555 208 L 555 250 L 398 250 L 398 212" fill="none" stroke="#c678dd" stroke-width="1.6" stroke-dasharray="5 3" marker-end="url(#f6-p)"/>
  <text x="476" y="270" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#c678dd">the orientation feeds back — it decides what I⁻¹ is in world space</text>
  <text x="476" y="286" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">(the linear row has no such loop: mass never turns)</text>
  <text x="25" y="312" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">Integrate in this order (momentum → velocity → position). That's semi-implicit Euler — stable, and what you already use.</text>
</svg>

The rows are identical except for one thing: **the angular row has a feedback loop.** `q`
determines `I⁻¹` in world space, and `I⁻¹` determines `ω`, which changes `q`. Mass never
turns, so the linear row is a straight line. That loop is the whole difference.

### Where the ½ω⊗q comes from

Don't memorize this — derive it once and it'll stick.

Over a tiny step `dt`, the body turns by angle `θ = |ω|·dt` about the axis `ω̂`. Write that
as a quaternion:

```
Δq = ( cos(θ/2),  ω̂ · sin(θ/2) )
```

`θ` is tiny, so `cos(θ/2) ≈ 1` and `sin(θ/2) ≈ θ/2`:

```
Δq ≈ ( 1,  ω̂ · |ω|·dt/2 )  =  ( 1,  ω·dt/2 )
```

Stack that onto the current orientation (ω is in world space, so it goes on the **left**):

```
q_new = Δq ⊗ q
      = ( 1, ½ω·dt ) ⊗ q
      = q  +  ½·(0,ω) ⊗ q · dt
```

Rearrange and there it is:

```
q̇ = ½ · (0, ω) ⊗ q
```

The ½ is the same half-angle from the sandwich. Nothing mysterious.

**Then normalize, every single step.** A quaternion has to sit exactly on the unit sphere;
Euler integration always steps slightly off it. Skip the normalize and your object slowly
inflates and shears instead of just rotating.

**Frame gotcha:** world-frame ω multiplies on the **left** (`½·ω⊗q`); body-frame ω
multiplies on the **right** (`½·q⊗ω`). Getting this backwards produces motion that looks
*almost* right, which makes it a nasty bug. Pick world frame and write it in a comment.

---

## 7. Where matrices come in — and where they don't

Now the question this whole file was building toward.

### The inertia problem, precisely

`I_body` is three constant numbers. But `L` and `ω` live in **world** space, and the body is
turning. So to compute `ω = I⁻¹L` you have to bridge the two frames:

```
I⁻¹_world  =  R · I⁻¹_body · Rᵀ
```

Read right to left: *rotate into body space → apply the three inertia numbers → rotate back
out.* The middle step is the one that needs a real linear map.

<svg viewBox="0 0 640 250" width="100%" style="max-width:640px;height:auto" role="img" aria-label="Three step pipeline: rotate L into body frame, scale by inverse inertia, rotate omega back to world">
  <defs>
    <marker id="f7-b" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#61afef"/></marker>
    <marker id="f7-r" markerWidth="9" markerHeight="7" refX="8" refY="3.5" orient="auto"><path d="M0,0 L9,3.5 L0,7 z" fill="#e06c75"/></marker>
  </defs>
  <rect x="18" y="72" width="112" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.5"/>
  <text x="74" y="96" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#e06c75">L</text>
  <text x="74" y="113" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">world frame</text>
  <line x1="130" y1="98" x2="192" y2="98" stroke="#61afef" stroke-width="2" marker-end="url(#f7-b)"/>
  <text x="161" y="60" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#61afef">q*.rotate()</text>
  <text x="161" y="88" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10" fill="#888">rotation</text>
  <rect x="196" y="72" width="112" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.5"/>
  <text x="252" y="96" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#e06c75">L</text>
  <text x="252" y="113" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">body frame</text>
  <line x1="308" y1="98" x2="370" y2="98" stroke="#d19a66" stroke-width="2.4" marker-end="url(#f7-r)"/>
  <text x="339" y="52" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#d19a66">× (1/I₁, 1/I₂, 1/I₃)</text>
  <text x="339" y="68" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10" font-weight="600" fill="#d19a66">non-uniform scale</text>
  <text x="339" y="88" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10" fill="#888">NOT a rotation</text>
  <rect x="374" y="72" width="112" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.5"/>
  <text x="430" y="96" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#61afef">ω</text>
  <text x="430" y="113" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">body frame</text>
  <line x1="486" y1="98" x2="548" y2="98" stroke="#61afef" stroke-width="2" marker-end="url(#f7-b)"/>
  <text x="517" y="60" text-anchor="middle" font-family="ui-monospace,monospace" font-size="11" fill="#61afef">q.rotate()</text>
  <text x="517" y="88" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="10" fill="#888">rotation</text>
  <rect x="552" y="72" width="76" height="52" rx="7" fill="none" stroke="#888" stroke-width="1.5"/>
  <text x="590" y="96" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="14" font-weight="600" fill="#61afef">ω</text>
  <text x="590" y="113" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11" fill="#888">world</text>
  <path d="M 74 140 L 74 168 L 517 168 L 517 140" fill="none" stroke="#61afef" stroke-width="1.2" stroke-dasharray="4 4" opacity="0.55"/>
  <text x="296" y="188" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" fill="#61afef">a quaternion can do both of these steps</text>
  <path d="M 339 140 L 339 210" fill="none" stroke="#d19a66" stroke-width="1.2" stroke-dasharray="4 4"/>
  <text x="339" y="228" text-anchor="middle" font-family="ui-sans-serif,system-ui,sans-serif" font-size="11.5" font-weight="600" fill="#d19a66">a quaternion can never do this one</text>
</svg>

### The hard limit on quaternions

Here is the precise reason quaternions can't cover everything:

> **`q ⊗ v ⊗ q*` always preserves length.** It is a rotation, by construction. `|v'| = |v|`,
> no exceptions.

But `ω = I⁻¹L` **changes** the vector's length *and* its direction, by different amounts in
different directions — that's the whole content of section 3. So no quaternion expression
can produce ω from L. It isn't a performance argument; it's outside what the type can say.

**A quaternion answers "which way is the body facing." The inertia tensor answers "how hard
is it to spin about each direction."** Different questions. The first needs 3 DOF; the
second needs a non-uniform scale, and there's no room for it in 4 constrained numbers.

### But you can factor it — so skip `Mat3` for now

`R · diag · Rᵀ` is an **eigendecomposition**: a scale wrapped in two rotations. You already
have a type that does rotations, and the scale is three multiplies. So walk the diagram
directly:

```cpp
void RigidBody::Recalculate()
{
    Velocity = Momentum * inverseMass;

    // 1. rotate L into body space, where inertia is just three numbers
    Vec3 Lbody = Orientation.conjugate().rotate(AngularMomentum);

    // 2. the non-uniform scale — the step no quaternion can express
    Vec3 wBody( Lbody.x * InvInertiaBody.x,
                Lbody.y * InvInertiaBody.y,
                Lbody.z * InvInertiaBody.z );

    // 3. rotate back out to world space
    AngularVelocity = Orientation.rotate(wBody);
}
```

`InvInertiaBody` is a plain `Vec3`. **No `Mat3`, no `toMatrix()`, no transpose, no matrix
inverse.** And it reads like the physics.

*Condition:* this needs `I_body` diagonal — i.e. body axes aligned with principal axes.
True for a sphere and for a centered box. An asymmetric compound shape would pick up
off-diagonal terms and need a real matrix (or a one-time diagonalization at setup, which is
the same eigendecomposition again).

### So when *is* a matrix better?

**When you apply `I⁻¹` more than once or twice per frame.** Rough costs:

| Operation | ~flops |
|---|---|
| `Mat3 × Vec3` | 15 |
| `Quaternion::rotate` (naive sandwich) | ~55 |
| `Quaternion::rotate` (optimized) | ~30 |
| The three-step version above (2 rotates) | ~60–110 |
| Building `I⁻¹_world` once | ~40–60 |

One application per frame: **quaternions win** — you skip the build cost entirely. But
collision resolution needs `I⁻¹(r × n)` inside the impulse denominator, and once there are
several contacts, or an iterative solver making multiple passes, you apply `I⁻¹` dozens of
times per body per frame. Then you build `I⁻¹_world` once and every use costs 15 instead of
60+. **Break-even is about two applications.**

Matrices also own three things outright:

1. **Anything that isn't a rotation** — scale, shear, projection. Inertia is the physics
   example; a quaternion structurally cannot represent these.
2. **Rendering.** GPUs consume 4×4 matrices. You convert at that boundary. (raylib's
   `rlRotatef` takes axis-angle, so you can dodge this for now — and *should*, because
   raylib's `Matrix` is column-major in memory despite being written row-wise in the struct,
   which is a guaranteed lost afternoon.)
3. **Readability when debugging.** A rotation matrix's three columns are literally where
   your X, Y, Z basis vectors ended up. Four quaternion floats tell you nothing at a glance.

### And what quaternions own outright

For **orientation state**, quaternions don't merely win on speed — they win on correctness:

| | Quaternion | Rotation matrix |
|---|---|---|
| Numbers stored | 4 | 9 |
| Constraints to maintain | **1** (unit length) | **6** (3 unit columns + 3 perpendicular) |
| Repair after integrating | 1 sqrt, 4 divides | Gram–Schmidt, every step |
| Failure mode when it drifts | slightly wrong angle | object visibly **shears** |
| Composition | 16 mults | 27 mults |
| Smooth interpolation | slerp | no sane equivalent |
| Size | 16 bytes | 36 bytes |

That constraint-count row is the real argument. Integrating a matrix directly pushes it off
the rotation manifold in six different ways at once; a quaternion can only fall off in one,
and you fix it with a normalize.

> ### The rule
>
> **Quaternions for orientation state, composition, and integration.
> Matrices for linear maps that aren't rotations — or for rotations applied many times and
> worth caching.**
>
> Inertia is the first kind. But because it's diagonal in body space, you can factor it
> through two quaternion rotations and defer `Mat3` until the solver starts iterating.

---

## 8. Common confusions

- **ω is not three separate spin rates.** One axis, one speed, written in coordinates.
- **`I` is not a scalar.** If you find yourself writing `AngularVelocity = L / inertia` with
  a float, you've lost the entire content of section 3 and your box will never tumble.
- **`L ∦ ω`** for anything that isn't a sphere. This is a feature, not a bug to fix.
- **Gravity produces zero torque.** If your body's spin changes in free flight, something is
  wrong.
- **Normalize the quaternion every step.** Not every few steps.
- **`q` and `−q` are the same rotation.** A sign flip in the debugger is not a bug.
- **World-frame ω goes on the left** of `q̇ = ½ω⊗q`; body-frame goes on the right.
- **`rotate()` takes only a vector.** The angle and axis are *already stored* in `w` and
  `vector` — that's what the quaternion is. A function that rotates a vector has signature
  `Vec3 → Vec3`; if yours takes an angle, you've conflated *constructing* a rotation with
  *applying* one.

---

## 9. Build order

Each step is independently verifiable. Don't skip ahead — every one of these is a place
where a silent bug hides behind plausible-looking motion.

| # | Step | How you know it works |
|---|---|---|
| 0 | Fix `Quaternion::rotate` → `Vec3 rotate(const Vec3&) const` | Rotate (1,0,0) by 90° about Y; **write the handedness in a comment** |
| 1 | `DrawBoxDebug(center, size, Quaternion)` in Visualizer | Hardcode `q = fromAxisAngle(t, axis)`; box spins. No physics involved |
| 2 | Add `Orientation`, `AngularVelocity`; integrate `q̇ = ½ω⊗q` | Constant hardcoded ω ⇒ dead-steady spin. Leave it running minutes: no drift, no shear |
| 3 | Add `InvInertiaBody` (Vec3), `AngularMomentum`; the 3-step `Recalculate()` | **Box tumbles, sphere doesn't** — with zero torque. This is the real test |
| 4 | `AddForceAtPoint(F, worldPoint)` → `torqueAcc += (p − Position) × F` | Drop a body: gravity is at the CoM, so spin must stay *exactly* constant |
| 5 | Contact point + normal in `IntersectionData` | — |
| 6 | Collision impulse with `v_p = v + ω × r`; friction impulse clamped to `μ·\|j\|` | A ball dropped with sidespin **rolls** instead of sliding — and you never coded "rolling" |
| 7 | `Mat3` + cached `I⁻¹_world` | Only when the solver starts iterating over multiple contacts |

For step 6, the impulse magnitude:

```
            −(1 + e) · (v_p · n)
   j = ─────────────────────────────────
        1/m  +  n · ( (I⁻¹(r × n)) × r )
```

The extra denominator term is *"how much of this impulse gets spent spinning the body
instead of stopping it."* Apply it as `Momentum += j·n` and `AngularMomentum += r × (j·n)`.
Then the same formula along the tangent, clamped to `μ·|j|` — that clamp is what makes
rolling **emerge** rather than being special-cased.

---

## 10. Map to the code

| Concept | Lives in |
|---|---|
| `Vec3`, cross product | `src/core/Vector.h` |
| Quaternion, `rotate`, `fromAxisAngle` | `src/core/Quaternion.h` |
| `Momentum`/`L` state, `Recalculate()`, integration | `src/core/RigidBody.{h,cpp}` |
| Torque from contact, impulse resolution | `src/Engine.cpp` — `ResolvePlaneCollision` |
| Contact point + normal | `src/core/IntersectionData.h` (needs both added) |
| Drawing orientation | `src/utils/Visualizer.{h,cpp}` |

**Known issues to fix along the way** (all in the current working tree):

- `RigidBody.h` — `inverseMass` is uninitialized until the first `Update()`, and
  `Recalculate()` recomputes `1/Mass` every frame. Compute it once in the constructor, and
  prefer storing the *inverse* so `0` cleanly means infinite mass.
- `Engine.cpp:34` — `Position.y -= data.IntersectionRadius` works only because
  `IntersectionRadius` is negative when penetrating *and* the plane happens to be Y-up. Two
  accidents. Use `Position += normal * (-surfaceDist)`.
- `Engine.cpp:32` — `Velocity.y < 0` carries the same Y-up assumption. Use
  `Velocity.dot(normal) < 0`.
- `IntersectionData` carries no contact point or normal — step 5 needs both.
- `Quaternion.h` — unqualified `cos` picks up the `double` overload; use `std::cos`.
