cost = 4
texture = "Stable"
ethnicity = ""

function getName()
	if language == "french" then
		return "Etables"
	else
		return "Stables"
	end
end

function getDescription()
	if language == "french" then
		return "Permet de former des cavaleries."
	else
		return "Allows to train cavalry."
	end
end

function getProducedUnits()
	return ethnicity.."Cavalry", 5
end

function isAllowed(town)
	return townHasBuilding(town, "Workshop") and townHasBuilding(town, "Barrack")
end
