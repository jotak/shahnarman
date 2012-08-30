cost = 4
texture = "Tavern"
ethnicity = ""

function getName()
	if language == "french" then
		return "Taverne"
	else
		return "Tavern"
	end
end

function getDescription()
	if language == "french" then
		return "+4 heureux et +25% de chances d'apparition d'un héros."
	else
		return "+4 happy and +25% chances of heroe apparition."
	end
end

function getMod_happy(val)
	return val + 4
end

function getMod_chances(val)
	return val * 1.25
end

function isAllowed(town)
	return townHasBuilding(town, "Granary")
end
