#!/usr/bin/env python
#range of ascii characters[0,127]

#to validate that there are no collisions in the mapping (we never map two different values into the same unsigned byte value)
ubyte_to_unicode_mapper = {}
unicode_to_ubyte_mapper = {}

heb_alphabet = u"םןףךץאבגדהוזחטיכלמנסעפצקרשת"
eng_alphabet_lwr_case = u"abcdefghijklmnopqrstuvwxyz"
eng_alphabet_high_case = u"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
numbers = u"0123456789"
symbols = u"!@#$%^&()-+.*/':;`\",[]\\=±“×׳?~ "
char_count = 0
#########################################################################
## The following are values i didn't manage to write in this code file

for c in heb_alphabet:
    ubyte_to_unicode_mapper[char_count] = c
    unicode_to_ubyte_mapper[c] = char_count
    char_count += 1
for c in eng_alphabet_lwr_case:
    ubyte_to_unicode_mapper[char_count] = c
    unicode_to_ubyte_mapper[c] = char_count
    char_count += 1
for c in eng_alphabet_high_case:
    ubyte_to_unicode_mapper[char_count] = c
    unicode_to_ubyte_mapper[c] = char_count
    char_count += 1
for c in numbers:
    ubyte_to_unicode_mapper[char_count] = c
    unicode_to_ubyte_mapper[c] = char_count
    char_count += 1
for c in symbols:
    ubyte_to_unicode_mapper[char_count] = c
    unicode_to_ubyte_mapper[c] = char_count
    char_count += 1


print "*** Dictionary size is %d. ***" % len(ubyte_to_unicode_mapper)

NN_Outputs_Num = 124

if NN_Outputs_Num < len(ubyte_to_unicode_mapper):
    print "Error! NN doesn't contain enough outputs to support current ubyte_to_unicode_mapper!!!"
    print "Current NN out num=%d" % NN_Outputs_Num
    print "ubyte_to_unicode_mapper size is %d" % len(ubyte_to_unicode_mapper)
    
print "Mapping Dictionary:"

"""for key in ubyte_to_unicode_mapper.iterkeys():
    #print "%d -> ([%s] %d times)" % (key, ubyte_to_unicode_mapper[key][0], ubyte_to_unicode_mapper[key][1])
    print "%d -> ([%s])" % (key, ubyte_to_unicode_mapper[key].encode('UTF-8'))
"""