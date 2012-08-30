cost = 4
texture = "Armory"
ethnicity = ""

function getName()
	if language == "french" then
		return "Armurerie"
	else
		return "Armory"
	end
end

function getDescription()
	if language == "french" then
		return "(TODO) améliore les caractéristiques des unités formées dans la ville."
	else
		return "(TODO) improve characteristics of units trained in this town."
	end
end

function isAllowed(town)
	return townHasBuilding(town, "Blacksmith")
end
