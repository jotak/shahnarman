cost = 4
texture = "Marketplace"
ethnicity = ""

function getName()
	if language == "french" then
		return "Marché"
	else
		return "Marketplace"
	end
end

function getDescription()
	if language == "french" then
		return "+2 heureux et +2 influence."
	else
		return "+2 happy and +2 influence."
	end
end

function getMod_happy(val)
	return val + 2
end

function getMod_radius(val)
	return val + 2
end

function isAllowed(town)
	return townHasBuilding(town, "Farm")
end
