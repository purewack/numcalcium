import sys
import re

def parseLD(content):
	regex_pattern = '(\w+)\s\=\s*(?:"([^"]*)"|(\S+))'

	shared_variables = {}

	matches = re.findall(regex_pattern, content)

	for m in matches:
		variable = m[0]
		address = m[2]
	    # Check if the key starts with "ulp_"
		if variable.startswith("ulp_var_"):
	        # Remove "ulp_var" from the beginning of the key
			variable = variable[8:].lower()
			address = address.strip()
			if address.endswith(';'):
				address = address[0:-1]

			shared_variables[variable.strip()] = int(address,16)

	return shared_variables

def generate(binary, ld, output):
	result = {}

	with open(binary,'rb') as a:
		result['binary'] = a.read()

	with open(ld,'r') as a:
		result['symbols'] = parseLD(a.read())

	with open(output,'w') as a:
		a.write("data=")
		print(result,file=a)


# Command-line argument handling
if len(sys.argv) != 4:
    print("Usage: python generate_ulp.py <input_binary> <input_ld> <output_py>")
else:
    _, bianry_path, ld_path, output_py = sys.argv
    generate(bianry_path, ld_path, output_py)

