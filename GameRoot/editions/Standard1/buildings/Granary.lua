cost = 4
texture = "Granary"
ethnicity = ""

function getName()
	if language == "french" then
		return "Grenier"
	else
		return "Granary"
	end
end

function getDescription()
	if language == "french" then
		return "+25% de nourriture."
	else
		return "+25% food."
	end
end

function getMod_growth(val)
	return val * 1.25
end

function isAllowed(town)
	return townHasBuilding(town, "Farm")
end
