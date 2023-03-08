import requests
import serial
import serial.tools.list_ports

# String Array of the barcode values
barcodes = []

# uncomment if you want to print a list of the available COM ports
"""
ports = serial.tools.list_ports.comports()
for port, desc, hwid in sorted(ports):
	print("{}: {} [{}]".format(port, desc, hwid))
"""
# Initializing UART communication through COM6
with serial.Serial() as ser:
	ser.baudrate = 19200
	ser.port = 'COM6'
	ser.open()
	ser.write(b'\n')
	ser.write(b'Barcode communication established....')
	ser.write(b'\n')

while 1:
	# Storing Barcode
	barcode = input("Enter barcode: ")
	# Adding barcode to list (array of strings)
	barcodes.append(barcode)

	# *Can be removed*
	# Prints the array of barcodes
	if barcode == "print":
		for barcodes in barcodes:
			print(barcodes)

	url = "https://barcode-monster.p.rapidapi.com/" + barcode

	headers = {
		"X-RapidAPI-Key": "ad2fc94709msh4c9e42920a5abecp198ba4jsn6160ecd3c90a",
		"X-RapidAPI-Host": "barcode-monster.p.rapidapi.com"
	}

	# Requesting ITEM data from API
	response = requests.request("GET", url, headers=headers)

	# Splitting string for parsing (Method 1) Can be deleted
	# res = response.text.split()
	try:
		# Parsing for Item Name
		sub1 = '"description": '
		sub2 = "(from"

		idx1 = response.text.index(sub1)
		idx2 = response.text.index(sub2)

		itemName = response.text[idx1 + len(sub1) + 1: idx2]
		print("Barcode 	: " + barcode)
		print("ItemName 	: " + itemName)
		print("\n")

		ser.open()
		#ser.write(str.encode(barcode))
		ser.write(str.encode(itemName))
		ser.close()

	except Exception:
		print("Item was not found within the API")

	# print("The list of words is : " + str(res))

	# Printing the results from API request
	# print(response.text)