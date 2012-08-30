cost = 4
texture = "ArcheryRange"
ethnicity = ""

function getName()
	if language == "french" then
		return "Terrain de tir"
	else
		return "Archery range"
	end
end

function getDescription()
	if language == "french" then
		return "Permet de former des archers."
	else
		return "Allows to train bowmen."
	end
end

function getProducedUnits()
	return ethnicity.."Bowmen", 4
end

function isAllowed(town)
	return townHasBuilding(town, "Workshop") and townHasBuilding(town, "Barrack")
end
