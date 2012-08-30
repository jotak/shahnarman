cost = 4
texture = "Sawmill"
ethnicity = ""

function getName()
	if language == "french" then
		return "Moulin"
	else
		return "Sawmill"
	end
end

function getDescription()
	if language == "french" then
		return "+10% de nourriture et +15% de production."
	else
		return "+10% food and +15% production."
	end
end

function getMod_food(val)
	return val * 1.1
end

function getMod_prod(val)
	return val * 1.15
end

function isAllowed(town)
	return townHasBuilding(town, "Workshop")
end
