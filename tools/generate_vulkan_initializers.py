import os

filename = os.environ['VULKAN_SDK'] + "/Include/vulkan/vulkan_core.h"
with open(filename, 'r') as file:
	lines = file.readlines()
	pass

types = {}
types2 = {}

typeEnumDefinitionLineIndex = -1

for i in range(len(lines)):
	if 'typedef struct' in lines[i] and 'VkStructureType' in lines[i + 1]:
		typename = lines[i].split()[2]
		key = typename.lower()
		types [key] = typename

	if 'typedef enum VkStructureType' in lines[i]:
		typeEnumDefinitionLineIndex = i

lineIndex = typeEnumDefinitionLineIndex + 1
while ';' not in lines[lineIndex]:
	enumname = lines[lineIndex].strip().split()[0]

	parts = enumname.lower().split('_')
	key = 'vk' + ''.join(parts[3:])

	if key in types:
		funcname = enumname.replace('_STRUCTURE_TYPE', '').lower()

		# Todo(Leo): Probably should add other extension suffixes as well
		if funcname[-3:] == 'khr':
			funcname = funcname[:-3] + funcname[-3:].upper()
		
		types2[key] = (types[key], enumname, funcname)

	lineIndex += 1

print("// Note(Computer): This file is generated by me, pls don't touch :)")
for key in types2:
	typename = types2[key][0]
	enumname = types2[key][1]
	funcname = types2[key][2]

	print ("static inline " + typename + " " + funcname + "(" + typename + " value = {}" ") { value.sType = " + enumname + "; return value; }")
