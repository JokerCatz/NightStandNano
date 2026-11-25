# adplug_scope_new(<target>): after the static library <target> is built, rename the operator new / delete symbols its objects reference
# (new, new[], nothrow forms, delete, delete[], sized forms; size_t is `j` on the RISC-V target and `m` on a 64-bit host) to adp_op_<mangled name>
# (adplug_c.cpp), so only AdPlug's allocations take its tracked path (per-handle list, 8 MiB per request / 16 MiB total caps, a
# failed request returns to the API call instead of abort()). No global operator new / delete is replaced: GME and the rest of the image keep
# the toolchain's. A check then fails the build if an operator new / delete reference is left undefined in the archive.
function(adplug_scope_new tgt)
  set(args)
  foreach(sz j m)
    foreach(op Znw Zna)
      list(APPEND args --redefine-sym _${op}${sz}=adp_op_${op}${sz} --redefine-sym _${op}${sz}RKSt9nothrow_t=adp_op_${op}${sz}RKSt9nothrow_t)
    endforeach()
    foreach(op ZdlPv ZdaPv)
      list(APPEND args --redefine-sym _${op}${sz}=adp_op_${op}${sz})
    endforeach()
  endforeach()
  foreach(op ZdlPv ZdaPv)
    list(APPEND args --redefine-sym _${op}=adp_op_${op} --redefine-sym _${op}RKSt9nothrow_t=adp_op_${op}RKSt9nothrow_t)
  endforeach()
  add_custom_command(TARGET ${tgt} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ${args} $<TARGET_FILE:${tgt}>
    COMMAND sh -c "! ${CMAKE_NM} -u $<TARGET_FILE:${tgt}> 2>/dev/null | grep -E ' U _Z(nw|na|dl|da)'"
    COMMENT "libadplug: operator new/delete scoped to AdPlug (adplug_scope.cmake)" VERBATIM)
endfunction()
