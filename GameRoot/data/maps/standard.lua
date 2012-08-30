-- Will be set by program
nbEthnicities = 0
nbPlayers = 0

-- Map parameters
nbParams = 5
nbParamsValues = { 5, 5, 5, 6, 4 }
--paramValues = {
--	{ 1, 2, 3, 4, 5 },		-- Map size
--	{ 1, 2, 3, 4, 5 },		-- Homogeneity
--	{ 1, 2, 3, 4, 5 },		-- Sea level
--	{ 1, 2, 3, 4, 5, 6 }	-- Towns density
--	{ 1, 2, 3, 4 }			-- Special tiles density
--}
paramDefaultValues = { 3, 4, 4, 3, 3 }
params = { 3, 4, 4, 3, 3 }	-- These params can be changed by program

-- Will be filled by this script using function "generate"
width = 30
height = 30
map = {}	-- 1-dim array representing terrain
towns = {}	-- 2-dim array representing list of towns ; 1 town is { position, ethnicity, size }
nbTowns = 0
players = {}
nbTemples = 0
temples = {}	-- 2-dim array representing list of temples ; 1 temple is { position, mana type, amount }
nbSpecTiles = 0
spectiles = {}	-- array representing list of special tiles positions (type of special tile is not managed here since it's edition specific.

function getName(language)
	if language == "french" then
		return "Standard"
	else
		return "Standard"
	end
end

function getDescription(language)
	if language == "french" then
		return "Génération aléatoire en spirale."
	else
		return "Random map generation in spiral."
	end
end

function getParamLabels(language)
	if language == "french" then
		return "Taille de la carte", "Homogénéité de la carte", "Niveau de la mer", "Densité des villes", "Densité des terrains spéciaux"
	else
		return "Map size", "Map homogeneity", "Sea level", "Towns density", "Special tiles density"
	end
end

availSizes = { 5, 15, 30, 50, 100 }
function getParamValueLabels(language)
	if language == "french" then
		return
			"Minuscule (5x5)", "Petite (15x15)", "Moyenne (30x30)", "Grande (50x50)", "Immense (100x100)",	-- Map size
			"Aucune", "Très faible", "Faible", "Moyenne", "Forte",				-- Homogeneity
			"Très bas", "Bas", "Moyen", "Haut", "Très haut",					-- Sea level
			"Aucune", "Très faible", "Faible", "Moyenne", "Forte", "Très forte",-- Towns density
			"Aucun", "Faible", "Moyenne", "Forte"								-- Special tiles density
	else
		return
			"Tiny (5x5)", "Small (15x15)", "Medium (30x30)", "Big (50x50)", "Huge (100x100)",	-- Map size
			"None", "Very weak", "Weak", "Average", "Strong",					-- Homogeneity
			"Very low", "Low", "Average", "High", "Very high",					-- Sea level
			"None", "Very weak", "Weak", "Average", "Strong", "Very strong",	-- Towns density
			"None", "Weak", "Average", "Strong"									-- Special tiles density
	end
end

-- Variable used in algo, got from map parameters
homogeneity = 1
seaLevel = 1
townsDensity = 1
spectilesDensity = 1

-- Variables or constants used for map generation
tile_unset = 0
tile_plain = 1
tile_forest = 2
tile_mountain = 3
tile_toundra = 4
tile_desert = 5
tile_sea = 6
NORTH = 1
EAST = 2
SOUTH = 3
WEST = 4
NE = 5
NW = 6
SE = 7
SW = 8
C = 9
NONE = 0
nbRegions = 0
algoData = {}
nbUnsetTiles = 0

function generate()
	math.randomseed(os.time())
	width = availSizes[params[1]]
	height = width
	homogeneity = params[2] - 1
	seaLevel = params[3] - 1
	townsDensity = (params[4] - 1) * 10
	spectilesDensity = params[5] - 1
	_createHotColdSpots()
	for i=1, width * height do
		map[i] = tile_unset
	end
	nbUnsetTiles = width * height
	_startGenerate()
	_generateMapObjects()
end

function _createHotColdSpots()
	rnd = math.random(8) - 1
	if rnd == 0 then
		mapHotSpot = SE
		mapColdSpot = NW
	elseif rnd == 1 then
		mapHotSpot = C
		mapColdSpot = NORTH
	elseif rnd == 2 then
		mapHotSpot = SOUTH
		mapColdSpot = C
	elseif rnd == 3 then
		mapHotSpot = SW
		mapColdSpot = NE
	elseif rnd == 4 then
		mapHotSpot = NE
		mapColdSpot = SW
	elseif rnd == 5 then
		mapHotSpot = NORTH
		mapColdSpot = C
	elseif rnd == 6 then
		mapHotSpot = C
		mapColdSpot = SOUTH
	elseif rnd == 7 then
		mapHotSpot = NW
		mapColdSpot = SE
	else
		mapHotSpot = C
		mapColdSpot = C
	end
end

function _startGenerate()
	-- first, create regions for players position
	nbRegionsPerSide = 2
	while nbPlayers > nbRegionsPerSide * nbRegionsPerSide do
		nbRegionsPerSide = nbRegionsPerSide + 1
	end
	nbRegions = nbRegionsPerSide * nbRegionsPerSide
	regionWidth = width / nbRegionsPerSide
	regionHeight = height / nbRegionsPerSide
	if regionWidth < 5 or regionHeight < 5 then
		-- regions are too small => players position are 100% random
		nbRegions = nbPlayers
		for i = 1, nbRegions do
			algoData[i] = {}
			algoData[i]["x"] = math.random(width)
			algoData[i]["y"] = math.random(height)
			players[2 * i - 1] = algoData[i]["x"]
			players[2 * i] = algoData[i]["y"]
			_generateTile(i, 1)
		end
	else
		-- in each region, put 0 or 1 player according to probabilities
		iPlayer = 1
		for i = 1, nbRegions do
			rx = (i-1) % nbRegionsPerSide
			ry = (i-1) / nbRegionsPerSide
			algoData[i] = {}
			algoData[i]["x"] = math.random(regionWidth * rx + 2, math.min(regionWidth * rx + regionWidth, width) - 1)
			algoData[i]["y"] = math.random(regionHeight * ry + 2, math.min(regionHeight * ry + regionHeight, height) - 1)
			-- Check if this region has a player (by calc. proba)
			-- Proba is the number of remaining players divided by the number of remaining regions (should never be greater than 1)
			proba = (nbPlayers + 1 - iPlayer) / (nbRegions + 1 - i)
			if nbPlayers - iPlayer == nbRegions - i or math.random() <= proba then
				players[2 * iPlayer - 1] = algoData[i]["x"]
				players[2 * iPlayer] = algoData[i]["y"]
				iPlayer = iPlayer + 1
				_generateTile(i, 1)
			else
				_generateTile(i, 0)
			end
		end
	end

	-- generation loop
	while nbUnsetTiles > 0 do
		dx, dy = _iterateSpiral()
		for i = 1, nbRegions do
--			print("Region", i, "; dir=", algoData[i]["dir"], "; x=", algoData[i]["x"], "; y=", algoData[i]["y"])
			algoData[i]["x"] = algoData[i]["x"] + dx
			algoData[i]["y"] = algoData[i]["y"] + dy
			-- generate tile if tile is valid
			if algoData[i]["x"] > 0 and algoData[i]["y"] > 0 and algoData[i]["x"] <= width and algoData[i]["y"] <= height then
				if map[algoData[i]["x"] + (algoData[i]["y"] - 1) * width] == tile_unset then
					_generateTile(i, 0)
				end
			end
		end
	end
end

-- Spiral data
_iDir = WEST
_iTurn = 1
_iSideLength = 1
_iNextTurn = 2 * _iSideLength

-- dir is the next direction
function _iterateSpiral()
	dx, dy = 0, 0
	if _iDir == NORTH then
		dy = -1
	elseif _iDir == EAST then
		dx = 1
	elseif _iDir == SOUTH then
		dy = 1
	elseif _iDir == WEST then
		dx = -1
	end

	if _iTurn == _iNextTurn then
		_iTurn = 1
		_iSideLength = _iSideLength + 1
		_iNextTurn = 2 * _iSideLength
		_iDir = 1 + _iDir % 4	-- rotate
	elseif _iTurn == _iSideLength then
		_iDir = 1 + _iDir % 4	-- rotate
		_iTurn = _iTurn + 1
	else
		_iTurn = _iTurn + 1
	end

	return dx, dy
end

function _generateTile(iRegion, bOnlyLand)
	x = algoData[iRegion]["x"]
	y = algoData[iRegion]["y"]

	seaScore = 2 * (5 - homogeneity)
	plainScore = seaScore
	forestScore = seaScore
	mountainScore = seaScore
	desertScore = seaScore
	toundraScore = seaScore
	modifier1 = 1
	modifier2 = 2 ^ homogeneity
	modifier3 = 3 ^ homogeneity

	for dx = -3, 3 do
		for dy = -3, 3 do
			if x+dx<1 or y+dy<1 or x+dx>width or y+dy>height then
				--continue
			else
				if dx == 3 or dy == 3 or dx == -3 or dy == -3 then
					modifier = modifier1
				elseif dx == 2 or dy == 2 or dx == -2 or dy == -2 then
					modifier = modifier2
				else
					modifier = modifier3
				end

				tile = map[(x+dx) + (y+dy-1) * width]
				if tile == tile_plain then
					plainScore = plainScore + modifier
				elseif tile == tile_forest then
					forestScore = forestScore + modifier
				elseif tile == tile_mountain then
					mountainScore = mountainScore + modifier
				elseif tile == tile_toundra then
					toundraScore = toundraScore + modifier
				elseif tile == tile_desert then
					desertScore = desertScore + modifier
				elseif tile == tile_sea then
					seaScore = seaScore + modifier
				end
			end
		end
	end

	seaMod = seaLevel / 3
	plainMod = 1.2
	forestMod = 0.8
	mountainMod = 0.8

	distance = _getDistanceFromSpot(mapHotSpot)
	desertMod = 0.3 + (width + height) / (distance + width + height)
	desertMod = desertMod * desertMod

	distance = _getDistanceFromSpot(mapColdSpot)
	toundraMod = 0.3 + (width + height) / (distance + width + height)
	toundraMod = toundraMod * toundraMod

	if bOnlyLand == 1 then
		seaScore = 0
	else
		seaScore = math.ceil(seaScore * seaMod)
	end
	plainScore = math.ceil(plainScore * plainMod)
	forestScore = math.ceil(forestScore * forestMod)
	mountainScore = math.ceil(mountainScore * mountainMod)
	desertScore = math.ceil(desertScore * desertMod)
	toundraScore = math.ceil(toundraScore * toundraMod)

	totalScore = plainScore + forestScore + mountainScore + desertScore + toundraScore + seaScore
	rnd = math.random(totalScore) - 1

	if map[x + width * (y-1)] == tile_unset then
		nbUnsetTiles = nbUnsetTiles - 1
	end

	if rnd < plainScore then
		map[x + width * (y-1)] = tile_plain
	elseif rnd < plainScore + forestScore then
		map[x + width * (y-1)] = tile_forest
	elseif rnd < plainScore + forestScore + mountainScore then
		map[x + width * (y-1)] = tile_mountain
	elseif rnd < plainScore + forestScore + mountainScore + desertScore then
		map[x + width * (y-1)] = tile_desert
	elseif rnd < plainScore + forestScore + mountainScore + desertScore + toundraScore then
		map[x + width * (y-1)] = tile_toundra
	else
		map[x + width * (y-1)] = tile_sea
	end
end

function _getDistanceFromSpot(spot)
	if spot == NW then
		return x + y
	elseif spot == NORTH then
		return math.abs(x - width / 2) + y
	elseif spot == NE then
		return (width - 1 - x) + y
	elseif spot == WEST then
		return x + math.abs(y - height / 2)
	elseif spot == EAST then
		return math.abs(y - height / 2) + width - x - 1
	elseif spot == SW then
		return x + height - 1 - y
	elseif spot == SOUTH then
		return math.abs(x - width / 2) + height - y - 1
	elseif spot == SE then
		return width + height - x - y - 2
	else
		return width + height;
	end
end

function _generateMapObjects()
	i = 0
	while i < width / 5 do
		j = 0
		while j < height / 5 do
			if math.random(100) < townsDensity then
				idx = 5*i + math.random(5) + width * (5*j + math.random(5) - 1)
				nbTowns = nbTowns + 1
				towns[nbTowns] = { idx, -1, math.random(2) }
			end
			if math.random(100) < 10 then
				idx = 5*i + math.random(5) + width * (5*j + math.random(5) - 1)
				nbTemples = nbTemples + 1
				temples[nbTemples] = { idx, math.random(4), math.random(5) }
			end
			if math.random(100) < spectilesDensity * 10 then
				idx = 5*i + math.random(5) + width * (5*j + math.random(5) - 1)
				nbSpecTiles = nbSpecTiles + 1
				spectiles[nbSpecTiles] = idx
			end
			if math.random(100) < spectilesDensity * 10 then	-- do it twice (more chances)
				idx = 5*i + math.random(5) + width * (5*j + math.random(5) - 1)
				if spectiles[nbSpecTiles] ~= idx then
					nbSpecTiles = nbSpecTiles + 1
					spectiles[nbSpecTiles] = idx
				end
			end
			j = j + 1 
		end
		i = i + 1
	end
end
