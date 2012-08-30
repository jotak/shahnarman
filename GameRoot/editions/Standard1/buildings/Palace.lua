cost = 4
texture = "Palace"
ethnicity = ""

function getName()
	if language == "french" then
		return "Palais"
	else
		return "Palace"
	end
end

function getDescription()
	if language == "french" then
		return "+2 heureux et +5 influence."
	else
		return "+2 happy and +5 influence."
	end
end

function getMod_happy(val)
	return val + 2
end

function getMod_radius(val)
	return val + 5
end

function isAllowed(town)
	return townHasBuilding(town, "CouncilHall")
end
