--[[ Power controller user script code.

The scripting implementation has changed, and is no longer compatible
with the older BASIC implementation. The most important changes are:

- Now Lua-based.
- No more line numbers, blocks of code identified by functions.
- Most of ON, OFF, etc. are kept as legacy functions, which can be called like
 e.g  ON(2345), ON("2345") or ON "2345", your choice.

Execution is still based on threads. Now threads are more visible and
manageable. Try starting some and you'll see them appearing in the
list.

Scripting samples are now available as snippets (below), separate from
the main script code. You can copy/paste them from/to the main script.

Stock snippets have names starting with 'default.'; changing or
creating snippets with such names is not recommended as your changes
may be erased on an upgrade.

]]--

function on1()
  outlet[1].state=on
  event.send("coil 1 energized",{coil_index=1,coil_state=true})
end
function off1()
  outlet[1].state=off
  event.send("coil 1 energized",{coil_index=1,coil_state=false})
end

-----
-----

function on2()
  outlet[2].state=on
  event.send("coil 2 energized",{coil_index=2,coil_state=true})
end
function off2()
  outlet[2].state=off
  event.send("coil 2 energized",{coil_index=2,coil_state=false})
end

-----
-----

function on3()
  outlet[3].state=on
  event.send("coil 3 energized",{coil_index=3,coil_state=true})
end
function off3()
  outlet[3].state=off
  event.send("coil 3 energized",{coil_index=3,coil_state=false})
end

-----
-----

function on4()
  outlet[4].state=on
  event.send("coil 4 energized",{coil_index=4,coil_state=true})
end
function off4()
  outlet[4].state=off
  event.send("coil 4 energized",{coil_index=4,coil_state=false})
end

-----
-----


function on5()
  outlet[5].state=on
  event.send("coil 5 energized",{coil_index=5,coil_state=true})
end
function off5()
  outlet[5].state=off
  event.send("coil 5 energized",{coil_index=5,coil_state=false})
end

-----
-----

function on6()
  outlet[6].state=on
  event.send("coil 6 energized",{coil_index=6,coil_state=true})
end
function off6()
  outlet[6].state=off
  event.send("coil 6 energized",{coil_index=6,coil_state=false})
end

-----
-----

function on7()
  outlet[7].state=on
  event.send("coil 7 energized",{coil_index=7,coil_state=true})
end
function off7()
  outlet[7].state=off
  event.send("coil 7 energized",{coil_index=7,coil_state=false})
end

-----
-----

function on8()
  outlet[8].state=on
  event.send("coil 8 energized",{coil_index=8,coil_state=true})
end
function off8()
  outlet[8].state=off
  event.send("coil 8 energized",{coil_index=8,coil_state=false})
end


